/*
 * ReconnectTimer.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef RECONNECTTIMER_H_
#define RECONNECTTIMER_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "timer/TimerDefs.h"

namespace vf_common
{

template<typename HandlerType, typename TimerType>
class ReconnectTimer : public TimerType
{
public:
    ReconnectTimer(HandlerType& handler, boost::asio::io_service& asio, bool isRelative = true)
    : TimerType(asio, isRelative)
    , _handler(handler)
    {

    }

    ~ReconnectTimer()
    {

    }

    bool onTimer()
    {
        _handler.connect();
        return false;
    }

private:
    HandlerType&    _handler;
};

}  // namespace vf_common


#endif /* RECONNECTTIMER_H_ */
