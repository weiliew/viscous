/*
 * FIXTraits.h
 *
 *  Created on: 10 Oct 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <logging/StdoutSink.h>
#include <logging/Log.h>
#include <signals/StaticSignal.h>
#include <buffers/MessageBuilder.h>
#include "apps/fix_engine/message/FIXMessagePoolDefs.h"
#include "signals/Signal.h"
#include "utilities/Utilities.h"
#include "apps/fix_engine/io/FIXInitiator.h"
#include "apps/fix_engine/io/FIXAcceptor.h"

#include <type_traits>

using namespace vf_common;

namespace vf_fix
{

template<typename PayloadType>
struct DefaultTapper
{
    void onData(PayloadType& payload)
    {
        // no-op
    }
};

template<typename Validate, unsigned int GroupCapacity = 50>
struct DefaultFIXTMsgraits
{
    // Note: include the desired fix version def files before including this header file
    typedef fix_defs::messages::Logon::SFIXMessage_Logon<Validate, GroupCapacity>           LogonMsgType;
    typedef fix_defs::messages::Logout::SFIXMessage_Logout<Validate, GroupCapacity>         LogoutMsgType;
    typedef fix_defs::messages::Heartbeat::SFIXMessage_Heartbeat<Validate, GroupCapacity>   HeartbeatMsgType;
};

struct DefaultFIXInitiatorTraits
{
    constexpr static size_t Capacity = 50;
    typedef std::true_type                                  Validate;
    typedef StdoutSink                                      LogSinkType;
    typedef Logger<LogSinkType>                             LoggerType;
    typedef LockFreeFIXMsg2k                                BufferPoolType;
    typedef typename BufferPoolType::BufferPtrType          BufferPtrType;
    typedef typename BufferPoolType::PooledFactoryType      PooledFactoryType;
    typedef std::true_type                                  InlineIOType;
    typedef DefaultFIXTMsgraits<Validate, Capacity>         MsgTraitsType;
    typedef DefaultTapper<BufferPtrType>                    TapperType;
    typedef Signal<BufferPtrType>                           OutgoingSignalType;
    typedef Signal<BufferPtrType>                           IncomingSignalType;

    typedef MessageBuilder<OutgoingSignalType,
                           PooledFactoryType,
                           InlineIOType>                    MsgBuilderType;

    typedef FIXInitiator<LoggerType,
                         BufferPoolType,
                         IncomingSignalType,
                         InlineIOType>                      SessionIoType;
};

struct DefaultFIXAcceptorTraits
{
    constexpr static size_t Capacity = 50;
    typedef std::true_type                                  Validate;
    typedef StdoutSink                                      LogSinkType;
    typedef Logger<LogSinkType>                             LoggerType;
    typedef LockFreeFIXMsg2k                                BufferPoolType;
    typedef typename BufferPoolType::BufferPtrType          BufferPtrType;
    typedef typename BufferPoolType::PooledFactoryType      PooledFactoryType;
    typedef std::true_type                                  InlineIOType;
    typedef DefaultFIXTMsgraits<Validate, Capacity>         MsgTraitsType;
    typedef DefaultTapper<BufferPtrType>                    TapperType;
    typedef Signal<BufferPtrType>                           OutgoingSignalType;
    typedef Signal<BufferPtrType>                           IncomingSignalType;

    typedef MessageBuilder<OutgoingSignalType,
                           PooledFactoryType,
                           InlineIOType>                    MsgBuilderType;

    typedef FIXAcceptor <LoggerType,
                         BufferPoolType,
                         IncomingSignalType,
                         InlineIOType>                      SessionIoType;
};

} // vf-fix


