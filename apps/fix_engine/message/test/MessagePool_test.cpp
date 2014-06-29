/*
 * MessagePool_test.cpp
 *
 *  Created on: 27 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE MessagePool_test
#include <sys/syscall.h>
#include <stdio.h>
#include <chrono>
#include <thread>

#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/asio/io_service.hpp>
#include "signals/Signal.h"
#include "buffers/MessageBuilder.h"

#include "apps/fix_engine/message/FIXMessagePoolDefs.h"
#include "apps/fix_engine/message/FIXMessageDecoder.h"

using namespace vf_common;
using namespace vf_fix;

template<typename FIXMsgType>
void testFIXMessage(FIXMsgType msg)
{
    FIXMessageDecoder<1024> decoder;
    BOOST_CHECK(msg->parseFIXMessage(decoder));
    BOOST_CHECK(msg->complete());
    BOOST_CHECK(msg->size() == 40);

    BOOST_MESSAGE("Decoded message: " << decoder.toString());
}

template<typename FIXMsgType>
class Sub
{
public:
    Sub()
    : _count(0)
    {

    }

    void onData(FIXMsgType& payload)
    {
        testFIXMessage(payload);
        ++_count;
    }

    int _count;
};


template<typename PoolDefs, typename InlineIO>
void runTest()
{
    // total of 10 FIX messages
    std::vector<std::string> inputArray = {
        std::string("8=FIX4.4") + (char) SOH + "9=",
        std::string("20") + (char) SOH + "35=D" + (char) SOH + "49=001" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH,
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=002" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH,
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=003" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=004" + (char) SOH,
        std::string("56=YYY") + (char) SOH + "10=123" + (char) SOH,
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=005" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=006" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=007" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH,
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=008" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=009" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=20" + (char) SOH + "35=D" + (char) SOH + "49=010" + (char) SOH + "56=YYY" + (char) SOH + "10=",
        std::string("1"),
        std::string("23"),
        std::string("") + (char) SOH + "gibberish"
    };

    // create the signals
    boost::asio::io_service io;
    boost::asio::io_service::work work(io);

    typedef typename PoolDefs::BufferPtrType        MessagePtrType;
    typedef Signal<MessagePtrType>                  SignalType;
    typedef typename PoolDefs::PooledFactoryType    FactoryType;

    FactoryType bufferFactory(0);

    SignalFactory<SignalType> signalFactory("SIG", io);
    auto inputSignal = signalFactory.create();
    auto outputSignal = signalFactory.create();

    FactoryType msgFactory;
    MessageBuilder<std::shared_ptr<SignalType>, FactoryType, InlineIO> msgBuilder(outputSignal, msgFactory);

    inputSignal->subscribe(&msgBuilder, 100);

    // create the subscriber
    Sub<MessagePtrType> sub;
    outputSignal->subscribe(&sub, 100);

    if(InlineIO::value == false)
    {
        inputSignal->run();
        outputSignal->run();
    }

    std::for_each(inputArray.begin(), inputArray.end(), [&inputSignal, &msgFactory](std::string msg){
        auto fixMsg = msgFactory.create();
        fixMsg->setBuffer(msg.data(), msg.size());
        if(InlineIO::value == true)
        {
            inputSignal->dispatch(fixMsg);
        }
        else
        {
            inputSignal->post(fixMsg);
        }
    });

    // check that we are getting all the messages in the end
    sleep(1);

    // check result
    BOOST_CHECK(sub._count == inputArray.size());


    BOOST_MESSAGE("Test finished with factory type: " << typeid(bufferFactory).name());
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE( MessagePool_test_1 )
{
    runTest<LockingFIXMsg2k, std::true_type>();
    runTest<LockingFIXMsg2k, std::false_type>();
}

BOOST_AUTO_TEST_CASE( MessagePool_test_2 )
{
    runTest<LockFreeFIXMsg2k, std::true_type>();
    runTest<LockFreeFIXMsg2k, std::false_type>();
}

