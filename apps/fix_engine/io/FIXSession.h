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

#include <type_traits>

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
    typedef typename FIXTraitsType::SessionIoType                       SessionIoType;
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
    , _recvSeqNum(0)
    , _sendSeqNum(0)
    {
        // register callbacks
        _sessionIo.registerConnectCallback(std::bind(&FIXSession<DerivedSessionType, FIXTraitsType>::onConnect, this));
        _sessionIo.registerDisconnectCallback(std::bind(&FIXSession<DerivedSessionType, FIXTraitsType>::onDisconnect, this));

        // set up incoming signal
        getIncomingSignal().subscribe(&_msgBuilder, 20);
        getIncomingSignal().subscribe(&_tapper, 10);

        // set up outgoing signal
        getOutgoingSignal().subscribe(this, 10);
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
        VF_LOG_DEBUG(_logger, "Received FIX message" << std::string(buffer->buffer(), buffer->size()));

        // check fix message and call onAppData if it is an application msg type and onAdminData if it is admin data type

    }

    void onConnect()
    {
        _derivedImpl.onConnect();
    }

    void onDisconnect()
    {
        _derivedImpl.onDisconnect();
    }

    void start()
    {
        _sessionIo.startConnectTimer();
        _sessionIo.getIO().run();
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

        if(!syncSendFIXMsg(_logonMsg))
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

    template<typename MsgType>
    void setCommonFields(MsgType& msg)
    {
        // TODO - Sender and Target Comp ID, SeqNum, Sending Time
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
    SessionIoType           _sessionIo;

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
    typename std::enable_if<MsgType::TYPE == fix_defs::messages::message_type::msg_0, void>::type
    addSeqNum(MsgType&)
    {
    }

    template<typename MsgType>
    typename std::enable_if<MsgType::TYPE != fix_defs::messages::message_type::msg_0, void>::type
    addSeqNum(MsgType& msg)
    {
        msg.header().setSubField(fix_defs::fields::SFIXField_MsgSeqNum<>::FID, ++_sendSeqNum);
    }

    template<typename MsgType, typename T = typename fix_defs::fields::SFIXField_CheckSum<>>
    typename std::enable_if<T::TYPE_NAME == fix_defs::fieldTypes::CHAR, void>::type
    setDummyCheckSum(MsgType& msg)
    {
        msg.trailer().setSubField(fix_defs::fields::SFIXField_CheckSum<>::FID, 0);
    }

    template<typename MsgType, typename T = typename fix_defs::fields::SFIXField_CheckSum<>>
    typename std::enable_if<T::TYPE_NAME == fix_defs::fieldTypes::STRING, void>::type
    setDummyCheckSum(MsgType& msg)
    {
        msg.trailer().setSubField(fix_defs::fields::SFIXField_CheckSum<>::FID, "000");
    }

    template<typename MsgType>
    size_t handlePreSend(MsgType& msg, boost::asio::const_buffer& toSend)
    {
        // TODO - checksum, seq num tracking etc
        msg.setSubField(fix_defs::fields::SFIXField_SenderCompID<>::FID, _senderCompID);
        msg.setSubField(fix_defs::fields::SFIXField_TargetCompID<>::FID, _targetCompID);
        if(!_senderSubID.empty())
        {
            msg.setSubField(fix_defs::fields::SFIXField_SenderSubID<>::FID, _senderSubID);
        }
        if(!_targetSubID.empty())
        {
            msg.setSubField(fix_defs::fields::SFIXField_TargetSubID<>::FID, _targetSubID);
        }

        addSeqNum(msg);
        setDummyCheckSum(msg);

        // get len and add checksum


        toSend = msg.getBufferOutput();
        return boost::asio::detail::buffer_size_helper(toSend);
    }

    DerivedSessionType&     _derivedImpl;

    State                   _state;

    OutgoingSignalType      _outgoingSignal;

    MsgBuilderType          _msgBuilder;
    TapperType              _tapper;

    // TODO - recover seq num and seq num negotiation - maybe have a specific class to handle seqnum
    size_t                  _recvSeqNum;
    size_t                  _sendSeqNum;

    // identity fields
    std::string             _senderCompID;
    std::string             _targetCompID;
    std::string             _senderSubID;
    std::string             _targetSubID;
};

} // vvf_fix



#endif /* FIXSESSION_H_ */
