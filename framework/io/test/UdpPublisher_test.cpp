/*
 * UdpPublisher_test.cpp
 *
 *  Created on: 4 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE UdpPublisher_test

#include <boost/test/included/unit_test.hpp>
#include <sys/syscall.h>
#include <stdio.h>
#include <thread>

#include "io/UdpPublisher.h"
#include "logging/Log.h"
#include "logging/StdoutSink.h"
#include "signals/Signal.h"
#include "buffers/PoolDefs.h"
#include "timer/TimerDefs.h"

using namespace vf_common;

template<typename PublisherType>
class PubTimer : public SecondsTimer
{
public:
    PubTimer(PublisherType& pub)
    : SecondsTimer(pub.getIO())
    , _pub(pub)
    , _count(0)
    {
    }

    ~PubTimer()
    {
    }

    bool onTimer()
    {
        ++_count;
        auto newMsg = _pub.getBufferFactory().create();

        std::ostringstream msg;
        msg << "Test pub message: " << _count;
        newMsg->setBuffer(msg.str().c_str(), msg.str().size());

        BOOST_TEST_MESSAGE("Sending message: " << msg.str());

        _pub.asyncWrite(newMsg);

        return false;
    }

private:
    PublisherType&  _pub;
    size_t          _count;
};

BOOST_AUTO_TEST_CASE( Publish_test_1 )
{
    BOOST_TEST_MESSAGE("Running test case Publish_test_1");

    StdoutSink sink;
    Logger<StdoutSink> myLogger(sink, LogDebug);

    typedef UdpPublisher<Logger<StdoutSink>, LockFreeFixedBuffer64, Signal<typename LockFreeFixedBuffer64::BufferPtrType>, std::false_type> UdpPublisherType;
    UdpPublisherType publisher(myLogger);
    PubTimer<UdpPublisherType> pubTimer(publisher);
    pubTimer.setInterval(2); // 2 seconds

    std::shared_ptr<std::thread> thread1 = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &publisher.getIO()));
    thread1->detach();

    BOOST_CHECK(publisher.start("239.255.0.1", boost::unit_test::framework::master_test_suite().argv[1], boost::unit_test::framework::master_test_suite().argv[2]));

    if(publisher.connected())
    {
        pubTimer.runTimer();
        BOOST_TEST_MESSAGE("Sleeping for 10 minutes");
        sleep(600);
    }

    publisher.getIO().stop();
    thread1->join();

    BOOST_TEST_MESSAGE("Test case Publish_test_1 completed");
}
