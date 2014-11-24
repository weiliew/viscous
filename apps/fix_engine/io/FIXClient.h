/*
 * FIXClient.h
 *
 *  Created on: 1 Sep 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXCLIENT_H_
#define FIXCLIENT_H_

#include "FIXTraits.h"
#include "FIXSession.h"

#include <type_traits>

using namespace vf_common;

namespace vf_fix
{

template<typename FIXTraitsType = DefaultFIXInitiatorTraits>
class FIXClient : public FIXSession<FIXClient<FIXTraitsType>, FIXTraitsType>
{
public:
    typedef FIXSession<FIXClient<FIXTraitsType>, FIXTraitsType> BaseType;

    // message type traits
    typedef typename FIXTraitsType::MsgTraitsType::LogonMsgType         LogonMsgType;
    typedef typename FIXTraitsType::MsgTraitsType::LogoutMsgType        LogoutMsgType;
    typedef typename FIXTraitsType::MsgTraitsType::HeartbeatMsgType     HeartbeatMsgType;

    FIXClient(const std::string& senderCompId,
            const std::string& targetCompId,
            const std::string& senderSubId = std::string(),
            const std::string& targetSubId = std::string())
    : BaseType(*this)
    {
        BaseType::setSenderCompID(senderCompId);
        BaseType::setTargetCompID(targetCompId);
        BaseType::setSenderSubID(senderSubId);
        BaseType::setTargetSubID(targetSubId);
    }

    virtual ~FIXClient()
    {
    }

    void onAppData(typename FIXTraitsType::BufferPtrType msg)
    {
    }

    bool setLogonMsg(LogonMsgType& msg)
    {
        BaseType::setCommonFields(msg);
        setupFIXMsg(msg);

        return true;
    }

    bool setLogoutMsg(LogoutMsgType& msg)
    {
        BaseType::setCommonFields(msg);
        setupFIXMsg(msg);

        return true;
    }

    bool setHeartbeatMsg(HeartbeatMsgType& msg)
    {
        BaseType::setCommonFields(msg);
        setupFIXMsg(msg);

        return true;
    }

    void onLogon()
    {
    }

    void onLogout()
    {
    }

    void onHeartBeat()
    {
    }

    void onConnect()
    {
        BaseType::logon();
    }

    void onDisconnect()
    {
    }

    void run(bool blocking = false)
    {
        if(_handlerThread)
        {
            return;
        }

        if(blocking)
        {
            BaseType::start();
        }
        else
        {
            _handlerThread.reset(new std::thread([this](){
                BaseType::start();
            }));
            _handlerThread->detach();
        }
    }

private:
    template<typename MsgType>
    void setupFIXMsg(MsgType& msg)
    {
        // set up any necessary fields here specific to the client
    }

    std::unique_ptr<std::thread>    _handlerThread;
};


} // namespace vf_fix

#endif /* FIXCLIENT_H_ */
