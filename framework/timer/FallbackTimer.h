/*
 * FallbackTimer.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FALLBACKTIMER_H_
#define FALLBACKTIMER_H_

#include "Timer.h"

namespace vf_common
{

template <typename IntervalType>
class FallbackTimer : public Timer<IntervalType>
{
public:
    FallbackTimer(boost::asio::io_service& asio, bool relative = true)
    : Timer<IntervalType> (asio, relative)
    , _intervalStep(0)
    , _intervalMax(0)
    , _origInterval(0)
    {

    }

    virtual ~FallbackTimer()
    {

    }

    virtual bool isValidTimer()
    {
        if(_intervalStep == IntervalType(0) || _intervalMax == IntervalType(0))
        {
            return false;
        }
        else
        {
            return Timer<IntervalType>::isValidTimer();
        }
    }

    void setFallbackInterval(uint64_t step, uint64_t max, uint64_t interval = 0)
    {
        _intervalStep = IntervalType(step);
        _intervalMax = IntervalType(max);

        if(interval)
        {
            _origInterval = IntervalType(interval);
            Timer<IntervalType>::setInterval(interval);
        }
    }

    using Timer<IntervalType>::setInterval;

    virtual void setInterval(uint64_t interval)
    {
        _origInterval = IntervalType(interval);

        if(_intervalStep == IntervalType(0) || _intervalMax == IntervalType(0))
        {
            setFallbackInterval(interval, interval);
        }

        Timer<IntervalType>::setInterval(interval);
    }

    virtual void onStop(typename Timer<IntervalType>::TimerStopCode stopCode = Timer<IntervalType>::TimerStopCode::Unknown)
    {
        Timer<IntervalType>::setInterval(_origInterval);
    }

    IntervalType getIntervalStep()
    {
        return _intervalStep;
    }

    IntervalType getMaxInterval()
    {
        return _intervalMax;
    }

protected:
    virtual void reCalcInterval()
    {
        IntervalType newIntv = Timer<IntervalType>::getInterval();
        newIntv += _intervalStep;
        if(newIntv > _intervalMax)
        {
            newIntv = _intervalMax;
        }
        Timer<IntervalType>::setInterval(newIntv);
    }

private:
    IntervalType    _intervalStep;
    IntervalType    _intervalMax;
    IntervalType    _origInterval;
};

}  // namespace vf_common


#endif /* FALLBACKTIMER_H_ */
