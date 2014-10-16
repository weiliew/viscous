/*
 * AutoIo_test.cpp
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#define BOOST_TEST_MODULE AutoIo_test

#include <boost/test/included/unit_test.hpp>
#include <sys/syscall.h>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <cstdlib>

#include <iostream>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>

#include "io/TcpInitiator.h"
#include "io/TcpAcceptor.h"
#include "io/TcpAcceptorHandler.h"
#include "logging/Log.h"
#include "logging/StdoutSink.h"
#include "signals/Signal.h"
#include "buffers/PoolDefs.h"

using namespace vf_common;

bool gEndTest = false;

struct IoTestPayload
{
    IoTestPayload()
    : _timePoint1(0)
    , _timePoint2(0)
    , _timePoint3(0)
    {
    }
    uint64_t   _timePoint1; // initiator send
    uint64_t   _timePoint2; // acceptor receive
    uint64_t   _timePoint3; // initiator receive
};


template<typename PayloadType, typename AcceptorType>
class SubAcceptor
{
public:
    SubAcceptor(size_t id, AcceptorType acceptor)
    : _id(id)
    , _acceptor(acceptor)
    , _lastSize(0)
    {
    }

    void onData(PayloadType& payload)
    {
        // get time now
        auto timeNow = std::chrono::high_resolution_clock::now();
        
        size_t payloadSize = payload->size() - _lastSize;
        _lastSize = 0;
        constexpr size_t singleSize = sizeof(IoTestPayload);
        const char * bufferLoc = payload->buffer();
        while(payloadSize >= singleSize)
        {
            // apply time to payload
            IoTestPayload * ioPayload = (IoTestPayload*)(payload->buffer()); // necessary evil
            IoTestPayload tempPayload(*ioPayload);

            tempPayload._timePoint2 = timeNow.time_since_epoch().count();
            payload->setBuffer((const char *)(&tempPayload), sizeof(IoTestPayload)); // necessary evil

            // send payload back to initiator
            auto payloadCopy = _acceptor->getBufferFactory().clone(payload);
            _acceptor->syncWrite(payloadCopy);

            bufferLoc += singleSize;
            payloadSize -= singleSize;
        }

        if(payloadSize > 0)
        {
            // partial message
            _lastSize = payloadSize;
        }
    }

private:
    size_t          _id;
    AcceptorType    _acceptor;
    size_t          _lastSize;
};

template<typename PayloadType, typename RecvObjType>
class NewAcceptorSub
{
public:
    void onData(PayloadType& payload)
    {
        BOOST_TEST_MESSAGE("Acceptor: Notification of new acceptor ...");

        auto& signal = payload->getCallbackSignal();
        signal.setName("AcceptorCallback");

        auto newSubscriber = std::make_shared<SubAcceptor<RecvObjType, PayloadType > >(_subscribers.size(), payload);
        _subscribers.push_back(newSubscriber);

        // set up callback for acceptor
        signal.subscribe(newSubscriber.get(), _subscribers.size());
    }

private:
    std::vector<std::shared_ptr<SubAcceptor<RecvObjType, PayloadType > > > _subscribers;
};

template<typename InitiatorType, typename PayloadType>
class InitiatorCallback : public MicrosecondsTimer
{
public:
    typedef boost::accumulators::accumulator_set<uint64_t,
            boost::accumulators::stats<boost::accumulators::tag::median,
                                       boost::accumulators::tag::min,
                                       boost::accumulators::tag::max> >  StatsType;

    InitiatorCallback(const std::string& name, InitiatorType& handle)
    : MicrosecondsTimer(handle.getIO())
    , _name(name)
    , _handle(handle)
    , _count(0)
    , _lastSize(0)
    {
    }

    void onData(PayloadType& payload)
    {
        // get time now
        auto timeNow = std::chrono::high_resolution_clock::now();

        size_t payloadSize = payload->size() - _lastSize;
        _lastSize = 0;
        constexpr size_t singleSize = sizeof(IoTestPayload);
        const char * bufferLoc = payload->buffer();
        while(payloadSize >= singleSize)
        {
            // apply time to payload
            IoTestPayload * ioPayload = (IoTestPayload*)(bufferLoc); // necessary evil
            ioPayload->_timePoint3 = timeNow.time_since_epoch().count();

            // get and store latency
            _e2eStats(ioPayload->_timePoint3 - ioPayload->_timePoint1);
            _i2aStats(ioPayload->_timePoint2 - ioPayload->_timePoint1);
            _a2iStats(ioPayload->_timePoint3 - ioPayload->_timePoint2);

            // print out
            ++_count;
            if(_count%1000 == 0)
            {
                BOOST_TEST_MESSAGE(_name << ": Msg received: " << _count);
            }

            bufferLoc += singleSize;
            payloadSize -= singleSize;

            // stop after count x
            if(_count > 5000 || gEndTest)
            {
                printStat("E2E", _e2eStats);
                printStat("I2E", _i2aStats);
                printStat("A2I", _a2iStats);
                gEndTest = true;
                _handle.disconnect(false);
                break;
            }
        }

        if(payloadSize > 0)
        {
            // partial message
            _lastSize = payloadSize;
        }
    }
    
    void printStat(const char * name, StatsType& stat)
    {
        BOOST_TEST_MESSAGE(_name << ": Latency Stat(" << name <<
        ") min:    " << boost::accumulators::extract_result<boost::accumulators::tag::min>(stat) <<
        " max:    " << boost::accumulators::extract_result<boost::accumulators::tag::max>(stat) <<
        " median: " << boost::accumulators::extract_result<boost::accumulators::tag::median>(stat));
    }
    
    bool onTimer()
    {
        // create and send message
        auto payload = _handle.getBufferFactory().create();

        // get time now
        auto timeNow = std::chrono::high_resolution_clock::now();
       
        // apply time to payload
        IoTestPayload tempPayload;
        tempPayload._timePoint1 = timeNow.time_since_epoch().count();
        payload->setBuffer((const char *)(&tempPayload), sizeof(IoTestPayload)); // necessary evil
        
        // send payload
        _handle.syncWrite(payload);
        
        // generate randon time for next timer interval within a millisecond
        size_t nextIntv = std::rand()%25;
        while(!nextIntv)
        {
            nextIntv = std::rand()%25;
        }
        setInterval(nextIntv);
        return false;
    }
    
    void onConnect()
    {
        BOOST_TEST_MESSAGE("onConnect callback - connected to endpoint");
        setInterval(1000000); // start after 1 second
        runTimer();
    }

    void onDisconnect()
    {
        BOOST_TEST_MESSAGE("onDisconnect");
        stopTimer();
    }

    std::string     _name;
    InitiatorType&  _handle;
    uint64_t        _count;
    StatsType       _e2eStats;
    StatsType       _i2aStats;
    StatsType       _a2iStats;
    size_t          _lastSize;
};

BOOST_AUTO_TEST_CASE( AutoIo_test_1 )
{
    const char * acceptorPort1 = "15001";
    const char * acceptorPort2 = "15002";
    
    BOOST_TEST_MESSAGE("Running test case AutoIo_test_1");

    boost::asio::io_service io;
    boost::asio::io_service::work myWork(io);

    boost::asio::io_service io2;
    boost::asio::io_service::work myWork2(io2);

    StdoutSink sink;
    Logger<StdoutSink> myLogger(sink, LogDebug);

    typedef TcpAcceptor<Logger<StdoutSink>, LockFreeFixedBuffer256, Signal<typename LockFreeFixedBuffer256::BufferPtrType>> TcpAcceptorType;
    typedef TcpAcceptorHandler<TcpAcceptorType> TcpAcceptorHandlerType;
    TcpAcceptorHandlerType tcpAcceptorHandler(io, myLogger);

    NewAcceptorSub<typename TcpAcceptorHandlerType::AcceptorPtrType, typename TcpAcceptorHandlerType::BufferPoolType::BufferPtrType> acceptorSub;
    tcpAcceptorHandler.newAcceptorSignal().subscribe(&acceptorSub);

    BOOST_TEST_MESSAGE("Starting acceptor on port: " << acceptorPort1);
    BOOST_CHECK(tcpAcceptorHandler.start("127.0.0.1", acceptorPort1));

    std::shared_ptr<std::thread> thread1 = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &io));
    thread1->detach();

    TcpAcceptorHandlerType tcpAcceptorHandler2(io2, myLogger);
    NewAcceptorSub<typename TcpAcceptorHandlerType::AcceptorPtrType, typename TcpAcceptorHandlerType::BufferPoolType::BufferPtrType> acceptorSub2;
    tcpAcceptorHandler2.newAcceptorSignal().subscribe(&acceptorSub2);

    BOOST_TEST_MESSAGE("Starting acceptor on port: " << acceptorPort2);
    BOOST_CHECK(tcpAcceptorHandler2.start("127.0.0.1", acceptorPort2));

    std::shared_ptr<std::thread> thread2 = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &io2));
    thread2->detach();

    sleep(1);
    
    // start the initiators
    typedef TcpInitiator<Logger<StdoutSink>, LockFreeFixedBuffer256, Signal<typename LockFreeFixedBuffer256::BufferPtrType>> InitiatorType;
    InitiatorType tcpInitiator1(myLogger);

    typedef InitiatorCallback<InitiatorType, LockFreeFixedBuffer256::BufferPtrType> InitiatorCallbackType;
    InitiatorCallbackType cb1("Initiator1", tcpInitiator1);
    tcpInitiator1.getCallbackSignal().subscribe(&cb1, 100);
    tcpInitiator1.registerConnectCallback(std::bind(&InitiatorCallbackType::onConnect, &cb1));
    tcpInitiator1.registerDisconnectCallback(std::bind(&InitiatorCallbackType::onDisconnect, &cb1));
    BOOST_CHECK(tcpInitiator1.start("127.0.0.1", acceptorPort1));
    std::thread initiatorThread1([&tcpInitiator1](){tcpInitiator1.getIO().run();});
    initiatorThread1.detach();

    InitiatorType tcpInitiator2(myLogger);
    InitiatorCallbackType cb2("Initiator2", tcpInitiator2);
    tcpInitiator2.getCallbackSignal().subscribe(&cb2, 100);
    tcpInitiator2.registerConnectCallback(std::bind(&InitiatorCallbackType::onConnect, &cb2));
    tcpInitiator2.registerDisconnectCallback(std::bind(&InitiatorCallbackType::onDisconnect, &cb2));
    BOOST_CHECK(tcpInitiator2.start("127.0.0.1", acceptorPort2));

    std::thread initiatorThread2([&tcpInitiator2](){tcpInitiator2.getIO().run();});
    initiatorThread2.detach();

    while(!gEndTest)
    {
        sleep(1);
    }
    
    BOOST_TEST_MESSAGE("Stopping test in 5 seconds");
    sleep(5);
    io.stop();
    io2.stop();
    tcpInitiator1.getIO().stop();
    tcpInitiator2.getIO().stop();

    sleep(1);

    BOOST_TEST_MESSAGE("Test case AutoIo_test_1 completed");
}


