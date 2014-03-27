/*
 * Signal_test.cpp
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#define BOOST_TEST_MODULE Signal_test
#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include "signals/Signal.h"
#include <sys/syscall.h>
#include <stdio.h>

using namespace vf_common;

struct PayloadType
{
    std::string _payload;
};

class Sub1
{
public:
    void onData(std::shared_ptr<PayloadType>& payload)
    {
        BOOST_TEST_MESSAGE("Received callback on Sub1 with payload: " << payload->_payload.c_str());
    }

    void onData(PayloadType* payload)
    {
        BOOST_TEST_MESSAGE("Received callback on Sub1 with payload: " << payload->_payload.c_str());
    }

private:
};

class Sub2
{
public:
    void onData(std::shared_ptr<PayloadType>& payload)
    {
        BOOST_TEST_MESSAGE("Received callback on Sub2 with payload: " << payload->_payload.c_str());
    }

    void onData(PayloadType* payload)
    {
        BOOST_TEST_MESSAGE("Received callback on Sub2 with payload: " << payload->_payload.c_str());
    }

private:
};

class Sub3
{
public:
    void onData(std::shared_ptr<PayloadType>& payload)
    {
        BOOST_TEST_MESSAGE("Received callback on Sub3 with payload: " << payload->_payload.c_str());
    }

    void onData(PayloadType* payload)
    {
        BOOST_TEST_MESSAGE("Received callback on Sub3 with payload: " << payload->_payload.c_str());
    }

private:
};


class SubPerf
{
public:
    void onData(std::shared_ptr<PayloadType>& payload)
    {
        // do something meaningfull
        int size=payload->_payload.size();
        PayloadType * payloadPtr = payload.get();
        for(int count=0;count<size;++count)
        {
            char c = payloadPtr->_payload.c_str()[count];
        }
    }

    void onData(PayloadType& payload)
    {
        // do something meaningfull
        int size=payload._payload.size();
        for(int count=0;count<size;++count)
        {
            char c = payload._payload.c_str()[count];
        }
    }

private:
};


BOOST_AUTO_TEST_CASE( Signal_test_1 )
{
    BOOST_TEST_MESSAGE("Running test case Signal_test_1");

    Sub1 sub1;
    Sub2 sub2;
    Sub3 sub3;

    std::shared_ptr<PayloadType> payload(new PayloadType());
    payload->_payload = "Test1";

    boost::asio::io_service io;
    boost::asio::io_service::work work(io);

    SignalFactory<Signal<PayloadType> > signalFactory("SIG1", io);
    std::shared_ptr<Signal<PayloadType> > mySignal1 = signalFactory.create();
    mySignal1->subscribe(&sub1, 100);
    mySignal1->subscribe(&sub2, 200);
    mySignal1->subscribe(&sub3, 300);

    BOOST_TEST_MESSAGE("NOTIFY 1");
    mySignal1->run();
    sleep(1);

    mySignal1->post(payload);
    mySignal1->run();
    sleep(1);

    mySignal1->unsubscribe(&sub2);
    payload->_payload = "Test2";
    BOOST_TEST_MESSAGE("NOTIFY 2");
    mySignal1->post(payload);

    mySignal1->run();
    sleep(1);

    mySignal1->subscribe(&sub2, 400);
    payload->_payload = "Test3";
    BOOST_TEST_MESSAGE("NOTIFY 3");
    mySignal1->post(payload);
    sleep(1);

    io.stop();
    BOOST_TEST_MESSAGE("Test case Signal_test_1 completed");
}

BOOST_AUTO_TEST_CASE( Signal_test_2 )
{
    BOOST_TEST_MESSAGE("Running test case Signal_test_2");

    SubPerf sub;

    std::shared_ptr<PayloadType> payload(new PayloadType());
    payload->_payload = "This is a performance test for callback function";

    boost::asio::io_service io;
    boost::asio::io_service::work work(io);
    Signal<PayloadType> mySignal("SIG", io);
    mySignal.subscribe(&sub, 100);

    BOOST_TEST_MESSAGE("NOTIFY 1");
    mySignal.run(3);
    sleep(1);


    struct timeval currTime;
    struct timeval endTime;
    int numTests = 10000000;

    BOOST_TEST_MESSAGE("Running perf test for POST method");

    gettimeofday(&currTime, NULL);
    for(int count=0;count<numTests;++count)
    {
        mySignal.post(payload);
    }
    gettimeofday(&endTime, NULL);

    uint64_t timediff = 1000000*(endTime.tv_sec - currTime.tv_sec) + (endTime.tv_usec - currTime.tv_usec);
    BOOST_TEST_MESSAGE("Time taken - post: " << timediff << " microseconds");
    BOOST_TEST_MESSAGE("Time taken - post(per message): " << (double) ((double)timediff/(double)numTests) << " microseconds");

    BOOST_TEST_MESSAGE("Running perf test for DISPATCH method");
    gettimeofday(&currTime, NULL);
    for(int count=0;count<numTests;++count)
    {
        mySignal.dispatch(payload);
    }
    gettimeofday(&endTime, NULL);

    timediff = 1000000*(endTime.tv_sec - currTime.tv_sec) + (endTime.tv_usec - currTime.tv_usec);
    BOOST_TEST_MESSAGE("Time taken - dispatch: " << timediff << " microseconds");
    BOOST_TEST_MESSAGE("Time taken - dispatch(per message): " << (double) ((double)timediff/(double)numTests) << " microseconds");

    BOOST_TEST_MESSAGE("Running perf test - CONTROL 1");
    gettimeofday(&currTime, NULL);
    for(int count=0;count<numTests;++count)
    {
        sub.onData(payload);
    }
    gettimeofday(&endTime, NULL);

    timediff = 1000000*(endTime.tv_sec - currTime.tv_sec) + (endTime.tv_usec - currTime.tv_usec);
    BOOST_TEST_MESSAGE("Time taken - control: " << timediff << " microseconds");
    BOOST_TEST_MESSAGE("Time taken - control(per message): " << (double) ((double)timediff/(double)numTests) << " microseconds");

    BOOST_TEST_MESSAGE("Running perf test - CONTROL 2");
    PayloadType payload2;
    payload2._payload = "This is a performance test for callback function";

    gettimeofday(&currTime, NULL);
    for(int count=0;count<numTests;++count)
    {
        sub.onData(payload2);
    }
    gettimeofday(&endTime, NULL);

    timediff = 1000000*(endTime.tv_sec - currTime.tv_sec) + (endTime.tv_usec - currTime.tv_usec);
    BOOST_TEST_MESSAGE("Time taken - control: " << timediff << " microseconds");
    BOOST_TEST_MESSAGE("Time taken - control(per message): " << (double) ((double)timediff/(double)numTests) << " microseconds");


    io.stop();
    BOOST_TEST_MESSAGE("Test case Signal_test_2 completed");
}

BOOST_AUTO_TEST_CASE( Signal_test_3 )
{
    BOOST_TEST_MESSAGE("Running test case Signal_test_3");

    Sub1 sub1;
    Sub2 sub2;
    Sub3 sub3;

    PayloadType* payload = new PayloadType();
    payload->_payload = "Test1";

    boost::asio::io_service io;
    boost::asio::io_service::work work(io);

    SignalFactory<Signal<PayloadType*> > signalFactory("SIG1", io);
    std::shared_ptr<Signal<PayloadType*> > mySignal1 = signalFactory.create();
    mySignal1->subscribe(&sub1, 100);
    mySignal1->subscribe(&sub2, 200);
    mySignal1->subscribe(&sub3, 300);

    BOOST_TEST_MESSAGE("NOTIFY 1");
    mySignal1->run();
    sleep(1);

    mySignal1->post(payload);
    mySignal1->run();
    sleep(1);

    mySignal1->unsubscribe(&sub2);
    payload->_payload = "Test2";
    BOOST_TEST_MESSAGE("NOTIFY 2");
    mySignal1->post(payload);

    mySignal1->run();
    sleep(1);

    mySignal1->subscribe(&sub2, 400);
    payload->_payload = "Test3";
    BOOST_TEST_MESSAGE("NOTIFY 3");
    mySignal1->post(payload);
    sleep(1);

    io.stop();

    delete payload;

    BOOST_TEST_MESSAGE("Test case Signal_test_1 completed");
}

