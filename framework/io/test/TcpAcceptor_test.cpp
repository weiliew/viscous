/*
 * TcpAcceptor_test.cpp
 *
 *  Created on: 9 Feb 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */


#define BOOST_TEST_MODULE TcpAcceptor_test

#include <boost/test/included/unit_test.hpp>
#include <sys/syscall.h>
#include <stdio.h>
#include <thread>

#include "io/TcpAcceptor.h"
#include "io/TcpAcceptorHandler.h"
#include "logging/Log.h"
#include "logging/StdoutSink.h"
#include "signals/Signal.h"
#include "buffers/PoolDefs.h"

using namespace vf_common;

template<typename PayloadType, typename AcceptorType>
class Sub1
{
public:
    Sub1(size_t id, AcceptorType acceptor)
    : _id(id)
    , _acceptor(acceptor)
    {

    }

    void onData(PayloadType& payload)
    {
        BOOST_TEST_MESSAGE("Acceptor: Received callback on Sub1 id: " << _id << " with payload: " << std::string(payload->buffer(), payload->size()));

        // send something back
        std::shared_ptr<PayloadType> message = std::make_shared<PayloadType>();
        _acceptor->syncWrite(std::string("sync: Received message: ") + std::string(payload->buffer(), payload->size()) + "\n");
        _acceptor->asyncWrite(std::string("async:  Received message: ") + std::string(payload->buffer(), payload->size()) + "\n");
    }

private:
    size_t          _id;
    AcceptorType    _acceptor;
};

template<typename PayloadType, typename RecvObjType>
class NewAcceptorSub
{
public:
    void onData(PayloadType& payload)
    {
        BOOST_TEST_MESSAGE("Acceptor: Notification of new acceptor ...");

        auto& signal = payload->getCallbackSignal();
        signal.setName("AcceptorCallback_");

        auto newSubscriber = std::make_shared<Sub1<RecvObjType, PayloadType>>(_subscribers.size(), payload);
        _subscribers.push_back(newSubscriber);

        // set up callback for acceptor
        signal.subscribe(newSubscriber.get(), _subscribers.size());
    }

private:
    std::vector<std::shared_ptr<Sub1<RecvObjType, PayloadType > > > _subscribers;
};


BOOST_AUTO_TEST_CASE( Accept_test_1 )
{
    BOOST_TEST_MESSAGE("Running test case Accept_test_1");

    boost::asio::io_service io;
    boost::asio::io_service::work myWork(io);

    std::shared_ptr<std::thread> thread1 = std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &io));

    StdoutSink sink;
    Logger<StdoutSink> myLogger(sink, LogDebug);

    typedef TcpAcceptorHandler<Logger<StdoutSink>, LockingFixedBuffer1k, Signal<typename LockingFixedBuffer1k::BufferPtrType>> TcpAcceptorHandlerType;
    TcpAcceptorHandlerType tcpAcceptorHandler(io, myLogger);

    NewAcceptorSub<typename TcpAcceptorHandlerType::TcpAcceptorPtrType, typename LockingFixedBuffer1k::BufferPtrType> acceptorSub;
    tcpAcceptorHandler.newAcceptorSignal().subscribe(&acceptorSub);

    // run a thread for the acceptor io
    std::thread acceptThread([&io](){io.run();});

    BOOST_CHECK(tcpAcceptorHandler.start("127.0.0.1", boost::unit_test::framework::master_test_suite().argv[1]));

    BOOST_TEST_MESSAGE("Sleeping for 10 minutes");
    sleep(600);

    io.stop();
    acceptThread.join();

    BOOST_TEST_MESSAGE("Test case Accept_test_1 completed");
}
