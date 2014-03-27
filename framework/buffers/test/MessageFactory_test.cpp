/*
 * MessageFactory_test.cpp
 *
 *  Created on: 21 Mar 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE MessageFactory_test
#include <sys/syscall.h>
#include <stdio.h>
#include <chrono>
#include <thread>

#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/asio/io_service.hpp>
#include "buffers/PoolDefs.h"

using namespace vf_common;

size_t gTestSize = 1000;

template<typename Collection>
class TestUser
{
public:
    TestUser(typename Collection::PooledFactoryType& collection)
    : _factory(collection)
    , _created(0)
    , _received(0)
    {
    }

    ~TestUser()
    {
    }

    void receive(typename Collection::BufferPtrType buffer)
    {
        ++_received;

        if(_received%100 == 0)
        {
            BOOST_MESSAGE("Received: " << _received);
        }

        // random wait time
        size_t nextIntv = std::rand()%25;
        while(!nextIntv)
        {
            nextIntv = std::rand()%25;
        }

        usleep(nextIntv);

        // put back into pool
        _factory.destroy(buffer);
    }

    typename Collection::BufferPtrType create()
    {
        ++_created;

        if(_created%100 == 0)
        {
            BOOST_MESSAGE("Created: " << _created);
        }

        return _factory.create();
    }

    bool done()
    {
        return (_received == gTestSize);
    }
private:
    typename Collection::PooledFactoryType& _factory;
    size_t _created;
    size_t _received;
};

template<typename FactoryType>
void runTest()
{
    // create and prime message factory
    typename FactoryType::PooledFactoryType bufferFactory(0);

    boost::asio::io_service io;
    boost::asio::io_service::work work(io);
    std::thread myThread([&io](){io.run();});
    myThread.detach();

    TestUser<FactoryType> producer(bufferFactory);
    TestUser<FactoryType> consumer(bufferFactory);
    for(size_t count=0;count<1000;++count)
    {
        auto payload = producer.create();
        io.post(boost::bind(&TestUser<FactoryType>::receive, &consumer, payload));
    }

    // test completed - allow time for work to finish
    // TODO - use promise and futures to get done condition
    while(!consumer.done())
    {
        sleep(1);
    }

    BOOST_MESSAGE("Test finished with queue size: " << bufferFactory.size());
    BOOST_CHECK(bufferFactory.size() == gTestSize);
}

BOOST_AUTO_TEST_CASE( MessageFactory_test_1 )
{
    runTest<LockingFixedBuffer64>();
}

BOOST_AUTO_TEST_CASE( MessageFactory_test_2 )
{
    runTest<LockFreeFixedBuffer64>();
}

