/*
 * FIXInitiator.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef FIXINITIATOR_H_
#define FIXINITIATOR_H_

#include <sys/syscall.h>
#include <stdio.h>
#include "io/TcpInitiator.h"
#include "fix/message/FIXMessage.h"
#include "signals/Signal.h"
#include "message/MessageFactory.h"
#include "utilities/Utilities.h"

using namespace vf_common;

namespace vf_fix
{

// TODO - may want to fix the use of  buffer pool and signal type
// FIXInitiator - contains a list of TCPInitiators - ability to connect to multiple end point - if required - at the same time
// registers callback functions for session handling and pass on messages to FIXMessageParser.

template<typename Logger, typename BufferPoolType, typename SignalType, typename InlineIO = std::false_type>
class FIXInitiator
{
public:
    typedef TcpInitiator<Logger, BufferPoolType, SignalType, InlineIO> InitiatorType;

    FIXInitiator()
    {

    }

    ~FIXInitiator()
    {

    }

    void processIncoming(boost::shared_ptr<FIXMessage> payload)
    {
        _outgoingSignal->notify(payload);
    }

private:
    std::vector<std::shared_ptr<InitiatorType>>  _initiatorList;
};

}  // namespace osf_fix


#endif /* FIXINITIATOR_H_ */
