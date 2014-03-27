/*
 * Timer_test.cpp
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE Timer_test
#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include "timer/TimerDefs.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <time.h>

using namespace vf_common;

class AbsoluteTimerTest : public SecondsTimer
{
public:
    AbsoluteTimerTest(boost::asio::io_service& asio)
    : SecondsTimer(asio, false)
    , _checkIntv(0)
    {
        clock_gettime(CLOCK_REALTIME, &_lastTriggerTime);
    }

    virtual ~AbsoluteTimerTest()
    {
    }

    bool onTimer()
    {
        timespec timeNow;
        clock_gettime(CLOCK_REALTIME, &timeNow);

        // check amount of time passed
        int numSecPassed = (timeNow.tv_sec - _lastTriggerTime.tv_sec) + (timeNow.tv_nsec - _lastTriggerTime.tv_nsec)/1000000000;
        BOOST_TEST_MESSAGE("onTimer called with real interval of: " << numSecPassed << " configured interval of: " << getInterval().total_seconds());

        BOOST_CHECK(numSecPassed == _checkIntv);

        _lastTriggerTime = timeNow;

        return false;
    }

    using SecondsTimer::setInterval;

    virtual void setInterval(uint64_t interval)
    {
        _checkIntv = interval;
        SecondsTimer::setInterval(interval);
    }

    void onStop(TimerStopCode stopCode)
    {
        BOOST_TEST_MESSAGE("onStop called with code: " << stopCode);
    }

private:
    timespec    _lastTriggerTime;
    int         _checkIntv;
};

class AbsoluteTimerFallbackTest : public SecondsFallbackTimer
{
public:
    AbsoluteTimerFallbackTest(boost::asio::io_service& asio)
    : SecondsFallbackTimer(asio, false)
    , _checkIntv(0)
    , _numMaxRun(3)
    {
        clock_gettime(CLOCK_REALTIME, &_lastTriggerTime);
    }

    virtual ~AbsoluteTimerFallbackTest()
    {
    }

    bool onTimer()
    {
        timespec timeNow;
        clock_gettime(CLOCK_REALTIME, &timeNow);

        // check amount of time passed
        int numSecPassed = (timeNow.tv_sec - _lastTriggerTime.tv_sec) + (timeNow.tv_nsec - _lastTriggerTime.tv_nsec)/1000000000;
        BOOST_TEST_MESSAGE("onTimer called with real interval of: " << numSecPassed << " configured interval of: " << getInterval().total_seconds());

        BOOST_CHECK(numSecPassed == _checkIntv);

        _checkIntv += getIntervalStep().total_seconds();
        if(_checkIntv > getMaxInterval().total_seconds())
        {
            _checkIntv = getMaxInterval().total_seconds();
            --_numMaxRun;
            if(_numMaxRun <= 0)
            {
                return true;
            }
        }

        _lastTriggerTime = timeNow;
        return false;
    }

    using SecondsFallbackTimer::setInterval;

    virtual void setInterval(uint64_t interval)
    {
        _checkIntv = interval;
        SecondsFallbackTimer::setInterval(interval);
    }

    void onStop(TimerStopCode stopCode)
    {
        BOOST_TEST_MESSAGE("onStop called with code: " << stopCode);
        BOOST_CHECK(stopCode == 0);
        exit(0);
    }

private:
    timespec    _lastTriggerTime;
    int         _checkIntv;
    int         _numMaxRun;;
};

BOOST_AUTO_TEST_CASE( Timer_Test_1 )
{
    BOOST_TEST_MESSAGE("Running test case Timer_Test_1");

    boost::asio::io_service asio;

    // repeating timer
    AbsoluteTimerTest timer1(asio);
    timer1.setInterval(3);
    timer1.runTimer();

    // fallback timer
    AbsoluteTimerFallbackTest timer2(asio);
    timer2.setInterval(3);
    timer2.setFallbackInterval(1, 5);
    timer2.runTimer();

    asio.run();

    while(1)
    {
        sleep(1);
    }

    BOOST_TEST_MESSAGE("Test case Timer_Test_1 completed");
}


