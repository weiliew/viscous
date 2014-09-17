/*
 * MessageBuilder_test.cpp
 *
 *  Created on: 10 May 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE MessageBuilder_test
#include <sys/syscall.h>
#include <stdio.h>
#include <thread>

#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/asio/io_service.hpp>
#include "buffers/MessageBuilder.h"
#include "buffers/BufferFactory.h"
#include "signals/Signal.h"

using namespace vf_common;

template<typename PayloadType>
class Sub
{
public:
    Sub()
    : _count(0)
    {

    }

    void onData(PayloadType& payload)
    {
        BOOST_TEST_MESSAGE("Received callback on subscriber with payload: " << payload->_buffer.c_str());
        BOOST_CHECK(payload->_buffer == "0123456789");
        ++_count;
    }

    size_t _count;
};


struct TestMessage
{
    TestMessage()
    {

    }

    TestMessage(const char * msg)
    : _buffer(msg)
    {

    }

    size_t getCompleteMsg()
    {
        // assume complete message is a 10 byte chunk
        if(_buffer.size() >= 10)
        {
            return 10;
        }
        else
        {
            return 0;
        }
    }

    const char * buffer()
    {
        return _buffer.data();
    }

    size_t size()
    {
        return _buffer.size();
    }

    void appendBuffer(const char * buffer, size_t len)
    {
        _buffer.append(buffer,len);
    }

    void setBuffer(const char * buffer, size_t len)
    {
        _buffer.assign(buffer,len);
    }

    void clear()
    {
        _buffer.clear();
    }

    void setSize(size_t size)
    {
        _buffer.resize(size);
    }

    std::string _buffer;
};

template<typename InlineIO = std::true_type>
void run_test()
{
    // Make sure we have same number of msgs in the array as the number of 'actual' decoded messages)
    std::vector<std::shared_ptr<TestMessage>> inputBuffer = {
            std::make_shared<TestMessage>("012345"),
            std::make_shared<TestMessage>("67890123456"),
            std::make_shared<TestMessage>("7890123456789"),
            std::make_shared<TestMessage>("0123"),
            std::make_shared<TestMessage>("456789"),
            std::make_shared<TestMessage>("0123456789"),
            std::make_shared<TestMessage>("012345678"),
            std::make_shared<TestMessage>("90123456789"),
            std::make_shared<TestMessage>("01234567890123456789")
    };

    // create the signals
    boost::asio::io_service io;
    boost::asio::io_service::work work(io);

    typedef Signal<std::shared_ptr<TestMessage>> SignalType;
    typedef SharedPtrBufferFactory<TestMessage>  FactoryType;

    SignalType inputSignal("SIG", io);
    SignalType outputSignal("SIG", io);

    FactoryType msgFactory;
    MessageBuilder<SignalType, FactoryType, InlineIO> msgBuilder(outputSignal, msgFactory);

    inputSignal.subscribe(&msgBuilder, 100);

    // create the subscriber
    Sub<std::shared_ptr<TestMessage>> sub;
    outputSignal.subscribe(&sub, 100);

    if(InlineIO::value == false)
    {
        inputSignal.run();
        outputSignal.run();
    }

    std::for_each(inputBuffer.begin(), inputBuffer.end(), [&inputSignal](std::shared_ptr<TestMessage> msg){
        if(InlineIO::value == true)
        {
            inputSignal.dispatch(msg);
        }
        else
        {
            inputSignal.post(msg);
        }
    });

    // check that we are getting all the messages in the end
    sleep(1);

    // check result
    BOOST_CHECK(sub._count == inputBuffer.size());
}

BOOST_AUTO_TEST_CASE( MessageBuilder_test_1 )
{
    run_test<std::true_type>();
    run_test<std::false_type>();
}

// TODO - raw pointer test

