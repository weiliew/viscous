/*
 * Io_test.cpp
 *
 *  Created on: 21 Feb 2014
 *      Author: Wei Liew [wei.liew@outlook.com]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#define BOOST_TEST_MODULE Io_test
#include <boost/test/included/unit_test.hpp>
#include <sys/syscall.h>
#include <stdio.h>
#include <thread>
#include "io/TcpInitiator.h"
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
        BOOST_TEST_MESSAGE("Initiator: Received callback on Sub1 with payload: " << std::string(payload->buffer(), payload->size()));
    }

private:
};

template<typename InitiatorType>
class InitiatorCallback
{
public:
    InitiatorCallback(InitiatorType& handle)
    : _handle(handle)
    {

    }

    void onConnect()
    {
        BOOST_TEST_MESSAGE("onConnect callback - connected to endpoint, starting write test");

        auto buffer1 = _handle.getBufferFactory().create();
        auto buffer2 = _handle.getBufferFactory().create();
        auto buffer3 = _handle.getBufferFactory().create();
        auto buffer4 = _handle.getBufferFactory().create();

        buffer1->setBuffer("ping sync 1");
        buffer2->setBuffer("ping sync 2");
        buffer3->setBuffer("ping async 1");
        buffer4->setBuffer("ping async 2");

        //std::cout << *buffer1;
        BOOST_TEST_MESSAGE("Sending buffer: " << std::string(buffer1->buffer(), buffer1->size()));
        _handle.asyncWrite(buffer1);
        usleep(100);

        BOOST_TEST_MESSAGE("Sending buffer: " << std::string(buffer2->buffer(), buffer2->size()));
        _handle.template asyncWrite<false>(buffer2);
        usleep(100);

        BOOST_TEST_MESSAGE("Async write test");

        BOOST_TEST_MESSAGE("Sending buffer: " << std::string(buffer3->buffer(), buffer3->size()));
        _handle.syncWrite(buffer3);
        usleep(100);

        BOOST_TEST_MESSAGE("Sending buffer: " << std::string(buffer4->buffer(), buffer4->size()));
        _handle.syncWrite(buffer4);
        usleep(100);
    }

    void onDisconnect()
    {
        BOOST_TEST_MESSAGE("onDisconnect");
    }

    InitiatorType& _handle;
};


BOOST_AUTO_TEST_CASE( TcpInitiator_Test_1 )
{
    BOOST_TEST_MESSAGE("Running test case TcpInitiator_Test_1");

    StdoutSink sink;
    Logger<StdoutSink> myLogger(sink, LogDebug);

    typedef TcpInitiator<Logger<StdoutSink>, LockingFixedBuffer1k, Signal<typename LockingFixedBuffer1k::BufferPtrType>> InitiatorType;
    InitiatorType tcpInitiator(myLogger);

    typedef InitiatorCallback<InitiatorType> InitiatorCallbackType;
    InitiatorCallbackType cb(tcpInitiator);
    tcpInitiator.registerConnectCallback(boost::bind(&InitiatorCallbackType::onConnect, &cb));
    tcpInitiator.registerDisconnectCallback(boost::bind(&InitiatorCallbackType::onDisconnect, &cb));

    Sub1<LockingFixedBuffer1k::BufferPtrType> sub1;
    tcpInitiator.getCallbackSignal().subscribe(&sub1, 100);
    BOOST_CHECK(tcpInitiator.start(boost::unit_test::framework::master_test_suite().argv[1], boost::unit_test::framework::master_test_suite().argv[2]));

    std::thread initiatorThread([&tcpInitiator](){tcpInitiator.getIO().run();});

    BOOST_TEST_MESSAGE("Sleeping for 10 minutes");
    sleep(600);

    tcpInitiator.getIO().stop();
    initiatorThread.join();

    BOOST_TEST_MESSAGE("Test case TcpInitiator_Test_1 completed");
}
