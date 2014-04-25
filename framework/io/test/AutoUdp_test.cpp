/*
 * AutoUdp_test.cpp
 *
 *  Created on: 25 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE AutoUdp_test

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

#include "io/UdpPublisher.h"
#include "io/UdpSubscriber.h"
#include "logging/Log.h"
#include "logging/StdoutSink.h"
#include "signals/Signal.h"
#include "buffers/PoolDefs.h"
#include "timer/TimerDefs.h"

using namespace vf_common;

struct IoTestPayload
{
    IoTestPayload()
    : _timePoint1(0)
    , _timePoint2(0)
    {
    }
    uint64_t   _timePoint1; // publisher send
    uint64_t   _timePoint2; // subscriber receive
};

static bool gEndTest = false;

template<typename PayloadType, typename SubscriberType>
class Subscriber
{
public:
    typedef boost::accumulators::accumulator_set<uint64_t,
            boost::accumulators::stats<boost::accumulators::tag::median,
                                       boost::accumulators::tag::min,
                                       boost::accumulators::tag::max> >  StatsType;

    Subscriber(size_t id, SubscriberType& handler)
    : _id(id)
    , _count(0)
    , _handler(handler)
    {
    }

    void onData(PayloadType& payload)
    {
        // get time now
        auto timeNow = std::chrono::high_resolution_clock::now();

        size_t payloadSize = payload->size();
        constexpr size_t singleSize = sizeof(IoTestPayload);
        const char * bufferLoc = payload->buffer();
        while(payloadSize >= singleSize)
        {
            // apply time to payload
            IoTestPayload * ioPayload = (IoTestPayload*)(bufferLoc); // necessary evil
            ioPayload->_timePoint2 = timeNow.time_since_epoch().count();

            // get and store latency
            _e2eStats(ioPayload->_timePoint2 - ioPayload->_timePoint1);

            // print out
            ++_count;
            if(_count%1000 == 0)
            {
                BOOST_TEST_MESSAGE("Subscriber " << _id << ": Msg received: " << _count);
            }

            bufferLoc += singleSize;
            payloadSize -= singleSize;

            // stop after count x
            if(_count > 5000 || gEndTest)
            {
                printStat("E2E", _e2eStats);
                gEndTest = true;
                _handler.disconnect(false);
                break;
            }
        }
    }

    void printStat(const char * name, StatsType& stat)
    {
        BOOST_TEST_MESSAGE("Subscriber " << _id << ": Latency Stat(" << name <<
        ") min:    " << boost::accumulators::extract_result<boost::accumulators::tag::min>(stat) <<
        " max:    "  << boost::accumulators::extract_result<boost::accumulators::tag::max>(stat) <<
        " median: "  << boost::accumulators::extract_result<boost::accumulators::tag::median>(stat));
    }

private:
    size_t          _id;
    StatsType       _e2eStats;
    uint64_t        _count;
    SubscriberType& _handler;
};

template<typename PublisherType>
class PubTimer : public MicrosecondsTimer
{
public:
    PubTimer(PublisherType& handle)
    : MicrosecondsTimer(handle.getIO())
    , _handle(handle)
    {
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

    PublisherType&  _handle;
};

BOOST_AUTO_TEST_CASE( AutoUdp_test_1 )
{
    const char * acceptorPort = "15003";
    const char * interface    = "0.0.0.0";
    const char * mcAddr       = "239.255.0.1";

    BOOST_TEST_MESSAGE("Running test case AutoUdp_test_1");

    boost::asio::io_service io;
    boost::asio::io_service::work myWork(io);

    boost::asio::io_service io2;
    boost::asio::io_service::work myWork2(io2);

    StdoutSink sink;
    Logger<StdoutSink> myLogger(sink, LogDebug);

    // start the subscribers (2x)

    typedef UdpSubscriber<Logger<StdoutSink>, LockFreeFixedBuffer256, Signal<typename LockFreeFixedBuffer256::BufferPtrType>, std::false_type> UdpSubscriberType;
    UdpSubscriberType subscriber1(myLogger);
    Subscriber<LockFreeFixedBuffer256::BufferPtrType, UdpSubscriberType> sub1(1, subscriber1);
    subscriber1.getCallbackSignal().subscribe(&sub1, 100);
    BOOST_CHECK(subscriber1.start(mcAddr, acceptorPort, interface));

    std::shared_ptr<std::thread> thread1 = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &subscriber1.getIO()));
    thread1->detach();

    UdpSubscriberType subscriber2(myLogger);
    Subscriber<LockFreeFixedBuffer256::BufferPtrType, UdpSubscriberType> sub2(2, subscriber2);
    subscriber2.getCallbackSignal().subscribe(&sub2, 100);
    BOOST_CHECK(subscriber2.start(mcAddr, acceptorPort, interface));

    std::shared_ptr<std::thread> thread2 = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &subscriber2.getIO()));
    thread2->detach();

    sleep(1);

    // start the publisher
    typedef UdpPublisher<Logger<StdoutSink>, LockFreeFixedBuffer256, Signal<typename LockFreeFixedBuffer256::BufferPtrType>, std::false_type> UdpPublisherType;
    UdpPublisherType publisher(myLogger);

    PubTimer<UdpPublisherType> pubTimer(publisher);
    publisher.registerConnectCallback(boost::bind(&PubTimer<UdpPublisherType>::onConnect, &pubTimer));
    publisher.registerDisconnectCallback(boost::bind(&PubTimer<UdpPublisherType>::onDisconnect, &pubTimer));

    std::shared_ptr<std::thread> thread3 = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &publisher.getIO()));
    thread3->detach();

    BOOST_CHECK(publisher.start(mcAddr, acceptorPort, interface));

    while(!gEndTest)
    {
        sleep(1);
    }

    BOOST_TEST_MESSAGE("Stopping test in 5 seconds");
    sleep(5);
    io.stop();
    io2.stop();
    publisher.getIO().stop();

    sleep(1);

    BOOST_TEST_MESSAGE("Test case AutoUdp_test_1 completed");
}


