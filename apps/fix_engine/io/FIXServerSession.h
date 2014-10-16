/*
 * FIXServerSession.h
 *
 *  Created on: 11 Oct 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXSERVERSESSION_H_
#define FIXSERVERSESSION_H_

#include "FIXServer.h"
#include "FIXSession.h"

using namespace vf_common;

namespace vf_fix
{

template<typename FIXTraitsType>
class FIXServerSession : public FIXSession<FIXServerSession<FIXTraitsType>, FIXTraitsType>
{
public:
    typedef typename FIXTraitsType::LoggerType          LoggerT;
    typedef typename FIXTraitsType::BufferPoolType      BufferPoolTypeT;
    typedef typename FIXTraitsType::OutgoingSignalType  SignalTypeT;
    typedef typename FIXTraitsType::InlineIOType        InlineIOT;

    typedef FIXSession<FIXServerSession<FIXTraitsType>, FIXTraitsType> BaseType;

    // message type traits
    typedef typename FIXTraitsType::MsgTraitsType::LogonMsgType         LogonMsgType;
    typedef typename FIXTraitsType::MsgTraitsType::LogoutMsgType        LogoutMsgType;
    typedef typename FIXTraitsType::MsgTraitsType::HeartbeatMsgType     HeartbeatMsgType;

    using BaseType::_sessionIo;

    FIXServerSession(typename FIXTraitsType::LoggerType&)
    : BaseType(*this)
    {
    }

    ~FIXServerSession()
    {
    }


    void onAppData(typename FIXTraitsType::BufferPtrType msg)
    {
    }

    bool setLogonMsg(LogonMsgType& msg)
    {
        setupFIXMsg(msg);

        return true;
    }

    bool setLogoutMsg(LogoutMsgType& msg)
    {
        setupFIXMsg(msg);

        return true;
    }

    bool setHeartbeatMsg(HeartbeatMsgType& msg)
    {
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
    }

    void onDisconnect()
    {
    }

    // functions required by the acceptor handler
    void onAccept(boost::asio::ip::tcp::endpoint endpoint)
    {
        _sessionIo.onAccept(endpoint);
    }

    typename BaseType::SessionIoType::ProtocolType::socket& getSocket()
    {
        return _sessionIo.getSocket();
    }

    void setDisconnectCallback(std::function<void (boost::asio::ip::tcp::endpoint&)> cb)
    {
        _sessionIo.setDisconnectCallback(cb);
    }

private:
    template<typename MsgType>
    void setupFIXMsg(MsgType& msg)
    {
        // TODO - set up comp ids etc

    }

};

} // vf_fix

#endif /* FIXSERVERSESSION_H_ */
