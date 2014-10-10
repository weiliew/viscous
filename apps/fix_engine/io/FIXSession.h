/*
 * FIXSession.h
 *
 *  Created on: 9 Aug 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXSESSION_H_
#define FIXSESSION_H_

using namespace vf_common;

namespace vf_fix
{

template<typename DerivedSessionType, typename FIXTraitsType>
class FIXSession
{
public:
    // There are 2 signals, incoming and outgoing
    // Incoming signal is internal and sends the message from IO -> Tapper -> MsgBuilder
    // Outgoing signal is for application use and it goes from MsgBuilder -> FIXSession -> Other Client App Subscribers
    // TODO - If client subscribes to signal, FIXSession is currently unable to block
    // or filter out certain messages - can be changed in Signal ...
    typedef typename FIXTraitsType::MsgBuilderType                      MsgBuilderType;
    typedef typename FIXTraitsType::OutgoingSignalType                  OutgoingSignalType;
    typedef typename FIXTraitsType::IncomingSignalType                  IncomingSignalType;
    typedef typename FIXTraitsType::InitiatorType                       InitiatorType;
    typedef typename FIXTraitsType::TapperType                          TapperType;
    typedef typename FIXTraitsType::LogSinkType                         LogSinkType;
    typedef typename FIXTraitsType::LoggerType                          LoggerType;
    typedef typename FIXTraitsType::BufferPtrType                       BufferPtrType;

    // message type traits
    typedef typename FIXTraitsType::MsgTraitsType::LogonMsgType         LogonMsgType;
    typedef typename FIXTraitsType::MsgTraitsType::LogoutMsgType        LogoutMsgType;
    typedef typename FIXTraitsType::MsgTraitsType::HeartbeatMsgType     HeartbeatMsgType;

    enum State
    {
        Unknown = 0,
        LoggingOn,
        LoggedOn,
        LoggingOut,
        LoggedOff
    };

    FIXSession(DerivedSessionType& derivedImpl)
    : _derivedImpl(derivedImpl)
    , _logSink()
    , _logger(_logSink, LogDebug) // Log level to be changed later if necessary
    , _sessionIo(_logger)
    , _state(Unknown)
    , _outgoingSignal("FIXSession_out", _sessionIo.getIO())
    , _msgBuilder(_outgoingSignal, _sessionIo.getBufferFactory())
    {
        // register callbacks
        _sessionIo.registerConnectCallback(std::bind(&FIXSession<DerivedSessionType, FIXTraitsType>::onConnect, this));
        _sessionIo.registerDisconnectCallback(std::bind(&FIXSession<DerivedSessionType, FIXTraitsType>::onDisconnect, this));
    }

    virtual ~FIXSession()
    {
        // make sure we have logged out (only if we are connected)
        if(_sessionIo.connected())
        {
            logout();
        }
    }

    OutgoingSignalType& getOutgoingSignal()
    {
        return _outgoingSignal;
    }

    OutgoingSignalType& getIncomingSignal()
    {
        return _sessionIo.getCallbackSignal();
    }

    bool setLogonMsg()
    {
        // prime the logon message
        return _derivedImpl.setLogonMsg(_logonMsg);
    }

    bool setLogoutMsg()
    {
        // prime the logout message
        return _derivedImpl.setLogoutMsg(_logoutMsg);
    }

    bool setHeartbeatMsg()
    {
        return _derivedImpl.setHeartbeatMsg(_hbMsg);
    }

    // called when a logon message has been received
    // return true to proceed, false to reject logon message
    bool onLogon()
    {
        return _derivedImpl.onLogon();
    }

    // called when a logout message is received
    void onLogout()
    {
        _derivedImpl.onLogout();
    }

    void onHeartBeat()
    {
        _derivedImpl.onHeartbeat();
    }

    // this is the main func where all received data is passed
    void onData(BufferPtrType& buffer)
    {
        // process data


    }

    void onConnect()
    {
        _derivedImpl.onConnect();
        logon();
    }

    void onDisconnect()
    {
        _derivedImpl.onDisconnect();
    }

    void run()
    {
        _sessionIo.startConnectTimer();
    }

    void addEndpoint(const std::string& host, const std::string& port)
    {
        _sessionIo.addEndpoint(host, port);
    }

    void removeEndPoint(const std::string& host, const std::string& port)
    {
        _sessionIo.removeEndpoint(host, port);
    }

protected:
    virtual void logon()
    {
        if(!setLogonMsg())
        {
            return;
        }

        if(syncSendFIXMsg(_logonMsg))
        {
            VF_LOG_ERROR(_logger, "Failed to send logon message. Disconnecting.");
            _sessionIo.disconnect();
        }
        else
        {
            VF_LOG_INFO(_logger, "Sent logon message.");
        }
    }

    virtual void logout(bool reconnect = false)
    {
        if(!setLogoutMsg())
        {
            return;
        }

        if(syncSendFIXMsg(_logoutMsg))
        {
            VF_LOG_WARN(_logger, "Failed to send logout message. Disconnecting with reconnect set to " << (reconnect ? "true" : "false"));
            _sessionIo.disconnect(reconnect);
        }
        else
        {
            VF_LOG_INFO(_logger, "Sent logout message. Disconnecting with reconnect set to " << (reconnect ? "true" : "false"));
            _sessionIo.disconnect(reconnect);
        }
    }

    /*virtual*/ void sendHeartBeat()
    {
        if(!setHeartbeatMsg())
        {
            return;
        }
        asyncSendFIXMsg(_hbMsg);
    }


    MsgBuilderType& getMsgBuilder()
    {
        return _msgBuilder;
    }

    TapperType& getTapper()
    {
        return _tapper;
    }

    LogonMsgType            _logonMsg;
    LogoutMsgType           _logoutMsg;
    HeartbeatMsgType        _hbMsg;

    LogSinkType             _logSink;
    LoggerType              _logger;
    InitiatorType           _sessionIo;

private:
    template<typename MsgType>
    bool syncSendFIXMsg(MsgType& msg)
    {
        boost::asio::const_buffer toSend;
        size_t bufSize = handlePreSend(msg, toSend);
        if(!bufSize)
        {
            return false;
        }

        return (_sessionIo.syncWrite(toSend) == bufSize);
    }

    template<typename MsgType>
    void asyncSendFIXMsg(MsgType& msg)
    {
        boost::asio::const_buffer toSend;
        if(!handlePreSend(msg, toSend))
        {
            return false;
        }

        _sessionIo.asyncWrite(toSend);
    }

    template<typename MsgType>
    size_t handlePreSend(MsgType& msg, boost::asio::const_buffer& toSend)
    {
        // TODO - checksum, seq num tracking etc

        toSend = msg.getBufferOutput();
        return boost::asio::detail::buffer_size_helper(toSend);
    }

    DerivedSessionType&     _derivedImpl;

    State                   _state;

    OutgoingSignalType      _outgoingSignal;

    MsgBuilderType          _msgBuilder;
    TapperType              _tapper;
};

} // vvf_fix



#endif /* FIXSESSION_H_ */
