/*
 * Buffers_test.cpp
 *
 *  Created on: 19 Jan 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */


#define BOOST_TEST_MODULE Buffers_test
#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/stack.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/asio/io_service.hpp>

#include "buffers/FixedBuffer.h"
#include "buffers/BufferPool.h"
#include "buffers/LockedQueue.h"
#include "buffers/LockFreeQueue.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <chrono>
#include <thread>

using namespace vf_common;

template<typename BufferType, typename PoolType>
class Producer
{
public:
    Producer(PoolType& pool, size_t numBuffer)
    : pool_(pool)
    {
        for(uint count=0;count<numBuffer;++count)
        {
            bufferList_.push_back(std::shared_ptr<BufferType>(bufferFactory_.create()));
        }
    }

    void run()
    {
        while(!bufferList_.empty())
        {
            auto data = bufferList_.back();
            bufferList_.pop_back();

            // insert time to the buffer
            auto tp = std::chrono::high_resolution_clock::now();
            *(uint64_t*)(data->buffer()) = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
            pool_.release(data);
        }
    }

private:
    PoolType&                                   pool_;
    std::vector<std::shared_ptr<BufferType> >   bufferList_;
    BufferFactory<BufferType>                   bufferFactory_;
};

template<typename BufferType, typename PoolType>
class Producer <BufferType *, PoolType>
{
public:
    Producer(PoolType& pool, size_t numBuffer)
    : pool_(pool)
    {
        for(uint count=0;count<numBuffer;++count)
        {
            bufferList_.push_back(bufferFactory_.create());
        }
    }

    void run()
    {
        while(!bufferList_.empty())
        {
            auto data = bufferList_.back();
            bufferList_.pop_back();

            // insert time to the buffer
            auto tp = std::chrono::high_resolution_clock::now();
            *(uint64_t*)(data->buffer()) = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
            pool_.release(data);
        }
    }

private:
    PoolType&                                   pool_;
    std::vector<BufferType *>                   bufferList_;
    BufferFactory<BufferType>                   bufferFactory_;
};


template<typename BufferType, typename PoolType>
class Consumer
{
public:
    Consumer(PoolType& pool, size_t testCount)
    : pool_(pool)
    , testCount_(testCount)
    {
        bufferList_.reserve(testCount);
    }

    ~Consumer()
    {
    }

    void run()
    {
        while(testCount_ > 0)
        {
            std::shared_ptr<BufferType> data;
            if(pool_.aquire(data))
            {
                // calculate receive time
                auto tp = std::chrono::high_resolution_clock::now();
                auto receiveTime = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();

                --testCount_;

                // get time and latency of the message
                uint64_t sendTime = *(uint64_t*)(data->buffer());
                latencyAccumulator_(receiveTime - sendTime);
                bufferList_.push_back(data);
            }
        }

        BOOST_TEST_MESSAGE("Consumer completed. Latency: " <<
                           " MIN [" << boost::accumulators::min(latencyAccumulator_) << "] " <<
                           " MAX [" << boost::accumulators::max(latencyAccumulator_) << "] " <<
                           " MED [" << boost::accumulators::median(latencyAccumulator_) << "] "
                           );
    }

private:
    PoolType&  pool_;
    int64_t    testCount_;
    std::vector<std::shared_ptr<BufferType> >   bufferList_;
    BufferFactory<BufferType>                   bufferFactory_;

    boost::accumulators::accumulator_set<uint64_t, boost::accumulators::features<boost::accumulators::tag::median,
                                                                                 boost::accumulators::tag::min,
                                                                                 boost::accumulators::tag::max>
                                                                                > latencyAccumulator_;
};


template<typename BufferType, typename PoolType>
class Consumer <BufferType *, PoolType>
{
public:
    Consumer(PoolType& pool, size_t testCount)
    : pool_(pool)
    , testCount_(testCount)
    {
        bufferList_.reserve(testCount);
        for_each(bufferList_.begin(), bufferList_.end(), [](BufferType* buffer){
            buffer = NULL;
        });
    }

    ~Consumer()
    {
        for_each(bufferList_.begin(), bufferList_.end(), [this](BufferType* buffer){
            if(buffer)
            {
                bufferFactory_.destroy(buffer);
            }
        });
    }

    void run()
    {
        while(testCount_ > 0)
        {
            BufferType * data = NULL;
            if(pool_.aquire(data))
            {
                // calculate receive time
                auto tp = std::chrono::high_resolution_clock::now();
                auto receiveTime = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();

                --testCount_;

                // get time and latency of the message
                uint64_t sendTime = *(uint64_t*)(data->buffer());
                latencyAccumulator_(receiveTime - sendTime);
                bufferList_.push_back(data);
            }
        }

        BOOST_TEST_MESSAGE("Consumer completed. Latency: " <<
                           " MIN [" << boost::accumulators::min(latencyAccumulator_) << "] " <<
                           " MAX [" << boost::accumulators::max(latencyAccumulator_) << "] " <<
                           " MED [" << boost::accumulators::median(latencyAccumulator_) << "] "
                           );
    }

private:
    PoolType&                   pool_;
    int64_t                     testCount_;
    std::vector<BufferType*>    bufferList_;
    BufferFactory<BufferType>                   bufferFactory_;

    boost::accumulators::accumulator_set<uint64_t, boost::accumulators::features<boost::accumulators::tag::median,
                                                                                 boost::accumulators::tag::min,
                                                                                 boost::accumulators::tag::max>
                                                                                > latencyAccumulator_;
};

template<typename PoolType, typename BufferType, size_t TestSize>
class TestCase
{
public:
    void runTest(const std::string& testName, size_t numConsumer, size_t numProducer)
    {
        if(numConsumer > numProducer)
        {
            numConsumer = numProducer;
        }

        BOOST_TEST_MESSAGE("Running test case " << testName << " Consumers: " << numConsumer << " Producers: " << numProducer);

        PoolType bufferPool;
        for(size_t count=0;count<numProducer;++count)
        {
            producerVec_.push_back(std::make_shared<Producer<BufferType, PoolType> >(bufferPool, TestSize));
        }

        // calculate consumer test size
        for(size_t count=0;count<numConsumer;++count)
        {
            auto consumer = std::make_shared<Consumer<BufferType, PoolType> >(bufferPool, TestSize);
            consumerVec_.push_back(consumer);

            threadList_.push_back(std::make_shared<std::thread>([consumer](){
                consumer->run();
            }));
        }

        for(size_t count=0;count<numProducer;++count)
        {
            threadList_.push_back(std::make_shared<std::thread>([this, count](){
                producerVec_[count]->run();
            }));
        }

        // wait for thread join
        for_each(threadList_.begin(), threadList_.end(), [](std::shared_ptr<std::thread> thread){
            thread->join();
        });

        BOOST_CHECK(true);

        BOOST_TEST_MESSAGE("Finished test case " << testName);
    }

private:
    std::vector<std::shared_ptr<Producer<BufferType, PoolType> > >  producerVec_;
    std::vector<std::shared_ptr<Consumer<BufferType, PoolType> > >  consumerVec_;
    std::vector<std::shared_ptr<std::thread> >                      threadList_;

};

template<typename PoolType, typename BufferType, size_t TestSize>
class TestCase<PoolType, BufferType*, TestSize>
{
public:
    void runTest(const std::string& testName, size_t numConsumer, size_t numProducer)
    {
        BOOST_TEST_MESSAGE("Running test case " << testName);

        PoolType bufferPool;
        for(size_t count=0;count<numProducer;++count)
        {
            producerVec_.push_back(std::make_shared<Producer<BufferType *, PoolType> >(bufferPool, TestSize));
        }

        for(size_t count=0;count<numConsumer;++count)
        {
            auto consumer = std::make_shared<Consumer<BufferType *, PoolType> >(bufferPool, TestSize);
            consumerVec_.push_back(consumer);

            threadList_.push_back(std::make_shared<std::thread>([consumer](){
                consumer->run();
            }));
        }

        for(size_t count=0;count<numProducer;++count)
        {
            threadList_.push_back(std::make_shared<std::thread>([this, count](){
                producerVec_[count]->run();
            }));
        }

        // wait for thread join
        for_each(threadList_.begin(), threadList_.end(), [](std::shared_ptr<std::thread> thread){
            thread->join();
        });

        BOOST_CHECK(true);

        BOOST_TEST_MESSAGE("Finished test case " << testName);
    }

private:
    std::vector<std::shared_ptr<Producer<BufferType *, PoolType> > >    producerVec_;
    std::vector<std::shared_ptr<Consumer<BufferType *, PoolType> > >    consumerVec_;
    std::vector<std::shared_ptr<std::thread> >                          threadList_;

};

BOOST_AUTO_TEST_CASE( Buffers_Test )
{
    typedef FixedBuffer<256>                                        BufferType;
    typedef LockedQueue<BufferType>                                 LQueue;
    typedef LockFreeQueue<BufferType>                               LFQueue;
    typedef BufferPool<LQueue>                                LockingPool;
    typedef BufferPool<LFQueue>                  LockFreePool;

    constexpr size_t TestSize = 100000;

    {
        TestCase<LockingPool, BufferType, TestSize> test;
        test.runTest("Buffers_LockedQueue_Test", 1, 1);
    }

    {
        TestCase<LockingPool, BufferType, TestSize> test;
        test.runTest("Buffers_LockedQueue_2x2_Test", 2, 2);
    }

    {
        TestCase<LockingPool, BufferType, TestSize> test;
        test.runTest("Buffers_LockedQueue_4x4_Test", 4, 4);
    }

    {
        TestCase<LockingPool, BufferType, TestSize> test;
        test.runTest("Buffers_LockedQueue_8x8_Test", 8, 8);
    }

    {
        TestCase<LockFreePool, BufferType*, TestSize> test;
        test.runTest("Buffers_LockFreeQueue_Test", 1, 1);
    }

    {
        TestCase<LockFreePool, BufferType*, TestSize> test;
        test.runTest("Buffers_LockFreeQueue_2x2_Test", 2, 2);
    }

    {
        TestCase<LockFreePool, BufferType*, TestSize> test;
        test.runTest("Buffers_LockFreeQueue_4x4_Test", 4, 4);
    }

    {
        TestCase<LockFreePool, BufferType*, TestSize> test;
        test.runTest("Buffers_LockFreeQueue_8x8_Test", 8, 8);
    }
}

