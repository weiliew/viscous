/*
 * TimerDefs.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef TIMERDEFS_H_
#define TIMERDEFS_H_

#include "Timer.h"
#include "FallbackTimer.h"

namespace vf_common
{

typedef Timer<boost::posix_time::seconds>               SecondsTimer;
typedef Timer<boost::posix_time::milliseconds>          MillisecondsTimer;
typedef Timer<boost::posix_time::microseconds>          MicrosecondsTimer;

typedef FallbackTimer<boost::posix_time::seconds>       SecondsFallbackTimer;
typedef FallbackTimer<boost::posix_time::milliseconds>  MillisecondsFallbackTimer;
typedef FallbackTimer<boost::posix_time::microseconds>  MicrosecondsFallbackTimer;

}  // namespace vf_common

#endif /* TIMERDEFS_H_ */
