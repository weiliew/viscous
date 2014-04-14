/*
 * UdpSubscriber_test.cpp
 *
 *  Created on: 4 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE UdpSubscriber_test

#include <boost/test/included/unit_test.hpp>
#include <sys/syscall.h>
#include <stdio.h>
#include <thread>

#include "io/UdpSubscriber.h"
#include "logging/Log.h"
#include "logging/StdoutSink.h"
#include "signals/Signal.h"
#include "buffers/PoolDefs.h"

using namespace vf_common;

template<typename PayloadType>
class Sub1
{
public:
    void onData(PayloadType& payload)
    {
        BOOST_TEST_MESSAGE("Subscriber: Received callback on Sub1 with payload: " << std::string(payload->buffer(), payload->size()));
    }

private:
};

BOOST_AUTO_TEST_CASE( Subscribe_test_1 )
{
    BOOST_TEST_MESSAGE("Running test case Subscribe_test_1");

    StdoutSink sink;
    Logger<StdoutSink> myLogger(sink, LogDebug);

    typedef UdpSubscriber<Logger<StdoutSink>, LockFreeFixedBuffer64, Signal<typename LockFreeFixedBuffer64::BufferPtrType>, std::false_type> UdpSubscriberType;
    UdpSubscriberType subscriber(myLogger);


    Sub1<LockFreeFixedBuffer64::BufferPtrType> sub1;
    subscriber.getCallbackSignal().subscribe(&sub1, 100);
    BOOST_CHECK(subscriber.start("239.255.0.1", boost::unit_test::framework::master_test_suite().argv[1], boost::unit_test::framework::master_test_suite().argv[2]));

    std::shared_ptr<std::thread> thread1 = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &subscriber.getIO()));
    thread1->detach();

    BOOST_TEST_MESSAGE("Sleeping for 10 minutes");
    sleep(600);

    subscriber.getIO().stop();
    thread1->join();

    BOOST_TEST_MESSAGE("Test case Subscribe_test_1 completed");
}



