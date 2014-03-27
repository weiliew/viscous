/*
 * Timer.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace vf_common
{

template <typename IntervalType>
class Timer
{
public:
    enum TimerStopCode
    {
        Completed = 0,
        Stopped,
        ErrorMisconfiguration,
        ErrorUnknown,
        Unknown
    };

    Timer(boost::asio::io_service& asio, bool relative = true)
    : _asio         (asio)
    , _timer        (asio)
    , _interval     (0)
    , _isRelative   (relative)
    , _isRunning    (false)
    {
    }

    virtual void setInterval(uint64_t interval)
    {
        _interval = IntervalType(interval);

        if(!isValidTimer())
        {
            stopTimer(ErrorMisconfiguration);
            return;
        }
    }

    virtual void setInterval(IntervalType interval)
    {
        _interval = interval;
    }

    virtual ~Timer()
    {
        onDestroy();
    }

    virtual void onDestroy()
    {
    }

    virtual void onStop(TimerStopCode stopCode = Unknown)
    {
    }

    // override by base class - return true if timer should be cancelled after calling this function, false to keep firing the timer on next interval
    virtual bool onTimer() = 0;

    void setRelative(bool relative)
    {
        _isRelative = relative;
    }

    void runTimer(bool immediate = false)
    {
        if(!isValidTimer())
        {
            onStop(ErrorMisconfiguration);
            return;
        }

        _isRunning = true;
        if(immediate)
        {
            if(onTimer())
            {
                _isRunning = false;
                onStop(Completed);
                return;
            }
        }

        // this call will cancel any unfinished timers that may be active
        _timer.expires_from_now(_interval);
        _timer.async_wait(boost::bind(&Timer<IntervalType>::onInternalTimer, this, boost::asio::placeholders::error));
    }

    void stopTimer(TimerStopCode stopCode = Stopped)
    {
        _timer.cancel();
        _isRunning = false;
        onStop(stopCode);
    }

    IntervalType getInterval()
    {
        return _interval;
    }

    bool isRunning()
    {
        return _isRunning;
    }

    virtual bool isValidTimer()
    {
        if(_interval == IntervalType(0))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

protected:
    virtual void reCalcInterval()
    {
    }

private:
    void onInternalTimer(const boost::system::error_code& error)
    {
        if (error)
        {
            onStop(ErrorUnknown);
            return;
        }

        if(_isRunning && !onTimer())
        {
            // continue to run - set up next timer
            reCalcInterval();

            if(!_isRelative)
            {
                // absolute time
                _timer.expires_at(_timer.expires_at() + IntervalType(_interval));
            }
            else
            {
                _timer.expires_from_now(IntervalType(_interval));
            }

            _timer.async_wait(boost::bind(&Timer<IntervalType>::onInternalTimer, this, boost::asio::placeholders::error));
        }
        else
        {
            stopTimer(Completed);
        }
    }

    boost::asio::io_service&    _asio;
    boost::asio::deadline_timer _timer;
    IntervalType                _interval;
    bool                        _isRelative;
    bool                        _isRunning;
};


}  // namespace vf_common

#endif /* TIMER_H_ */
