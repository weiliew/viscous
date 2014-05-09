/*
 * FIXAcceptor.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef FIXACCEPTOR_H_
#define FIXACCEPTOR_H_

#include <sys/syscall.h>
#include <stdio.h>
#include "io/TcpAcceptor.h"
#include "fix/message/FIXMessage.h"
#include "signals/Signal.h"
#include "message/MessageFactory.h"
#include "utilities/Utilities.h"
#include "FIXSessionHandler.h"

using namespace osf_common;

namespace osf_fix
{

template<typename SessionHandlerType>
class FIXAcceptor : public TcpAcceptor<BasicMessageFactory<FIXMessage>, SignalFactory<Signal<FIXMessage> > >
{
public:
    FIXAcceptor(boost::asio::io_service& ioService, Logger& logger, boost::shared_ptr<typename SignalFactory<Signal<FIXMessage> >::ProductType> signal)
    : TcpAcceptor<BasicMessageFactory<FIXMessage>, SignalFactory<Signal<FIXMessage> > > (ioService, logger, signal)
    , _sessionHandler (logger)
    {
    }

    virtual ~FIXAcceptor()
    {

    }

    // override to perform session management
    virtual void processIncoming(boost::shared_ptr<FIXMessage> payload)
    {
        // parse the payload into a FIX message
        _sessionHandler.parseFIXMessage(payload);

        //_outgoingSignal->notify(payload);
    }

private:
    SessionHandlerType _sessionHandler;
};

}  // namespace osf_fix


#endif /* FIXACCEPTOR_H_ */
