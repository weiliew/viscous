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

using namespace vf_common;

namespace vf_fix
{

template<typename Logger, typename BufferPoolType, typename SignalType, typename InlineIO = std::false_type>
class FIXAcceptor : public TcpAcceptor<Logger, BufferPoolType, SignalType, InlineIO>
{
public:
    typedef TcpAcceptor<Logger, BufferPoolType, SignalType, InlineIO> BaseType;
    typedef Logger          LoggerT;
    typedef BufferPoolType  BufferPoolTypeT;
    typedef SignalType      SignalTypeT;
    typedef InlineIO        InlineIOT;

    using BaseType::ProtocolType;

    FIXAcceptor(Logger& logger)
    : BaseType(logger)
    {
    }

    virtual ~FIXAcceptor()
    {

    }

    virtual void disconnect(bool reconnect = true)
    {
        BaseType::disconnect(reconnect);
    }



private:
};

}  // namespace vf_fix


#endif /* FIXACCEPTOR_H_ */
