/*
 * TcpInitiator.h
 *
 *  Created on: 3 Feb 2014
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef TCPINITIATOR_H_
#define TCPINITIATOR_H_

#include "IO.h"
#include "ReconnectTimer.h"

namespace vf_common
{

template<typename Logger, typename BufferPoolType, typename SignalType, typename InlineIO = std::false_type>
class TcpInitiator : public IO<Logger, BufferPoolType, SignalType, boost::asio::ip::tcp, InlineIO>
{
public:
    typedef IO<Logger, BufferPoolType, SignalType, boost::asio::ip::tcp, InlineIO> BaseType;

    using BaseType::_logger;
    using BaseType::_io;
    using BaseType::_socket;
    using BaseType::_lastEndpoint;

    TcpInitiator(Logger& logger)
    : BaseType(logger)
    , _reconnectTimer(*this, _io)
    , _reconnect(true)
    , _resolver(_io)
    , _connected(false)
    {
        // TODO - make configurable
        _reconnectTimer.setFallbackInterval(1,5,1);
    }

    virtual ~TcpInitiator()
    {
    }

    void startConnectTimer()
    {
        if(!_reconnectTimer.isRunning())
        {
            _reconnectTimer.runTimer();
        }
    }

    bool connect()
    {
        if(_socket.is_open())
        {
            return true;
        }

        if(_lastEndPointIter == boost::asio::ip::tcp::resolver::iterator())
        {
            // no end point
            return false;
        }

        boost::asio::ip::tcp::endpoint endpoint = *_lastEndPointIter;
        VF_LOG_INFO(_logger, "Attempting to connect to endpoint: " << endpoint.address().to_string() << ":" << endpoint.port());

        boost::system::error_code errCode;
        _socket.open(endpoint.protocol(), errCode);
        if(errCode)
        {
            VF_LOG_WARN(_logger, "Failed to open socket with error: " << errCode.message());
            return false;
        }

        _socket.async_connect(endpoint, boost::bind(&TcpInitiator<Logger, BufferPoolType, SignalType, InlineIO>::handleConnect, this, boost::asio::placeholders::error));

        return true;
    }

    void disconnect(bool reconnect = true)
    {
        if(_socket.is_open())
        {
            _socket.close();
        }

        _reconnect = reconnect;
        onDisconnect();
    }

    virtual void onConnect()
    {
        _lastEndpoint = *_lastEndPointIter;

        // TODO - config
        _socket.set_option(boost::asio::ip::tcp::no_delay(true)); // disable nagles
        _socket.set_option(boost::asio::socket_base::receive_buffer_size(16777216)); // 16MB buffer size

        BaseType::onConnect();
        _reconnectTimer.stopTimer();
        _connected = true;
    }

    virtual void onDisconnect()
    {
        BaseType::onDisconnect();
        _connected = false;
        if(_reconnect)
        {
            startConnectTimer();
        }
    }

    virtual bool setHostPort(const std::string& host, const std::string& port)
    {
        try
        {
            _lastEndPointIter = _resolver.resolve(boost::asio::ip::tcp::resolver::query(host, port));
            _endPointIter = _lastEndPointIter;
        }
        catch (boost::system::system_error& e)
        {
            VF_LOG_WARN(_logger, "Failed to resolve endpoint with host:port = " << host << ":" << port << " [" << e.what() << "]");
            return false;
        }

        return true;
    }

    virtual boost::asio::ip::tcp::endpoint getLastEndpoint()
    {
        return *_lastEndPointIter;
    }

    bool start(const std::string& host, const std::string& port)
    {
        try
        {
            VF_LOG_INFO(_logger, "Starting initiator for host:port = " << host << ":" << port);
            return (setHostPort(host, port) && connect());
        }
        catch (boost::system::system_error& e)
        {
            VF_LOG_WARN(_logger, "Failed to initialise initiator host:port = " << host << ":" << port << " [" << e.what() << "]");
            return false;
        }

        return false;
    }

    bool connected()
    {
        return _connected;
    }

    void setReconnect(bool reconnect)
    {
        _reconnect = reconnect;
    }

private:
    void handleConnect(const boost::system::error_code& error)
    {
        if (!error)
        {
            onConnect();
            BaseType::asyncRead(*_lastEndPointIter);
        }
        else
        {
            boost::asio::ip::tcp::endpoint oldendpoint = *_lastEndPointIter;
            if((++_lastEndPointIter) != boost::asio::ip::tcp::resolver::iterator())
            {
                boost::asio::ip::tcp::endpoint endpoint = *_lastEndPointIter;
                VF_LOG_INFO(_logger, "Attempting to connect to alternative endpoint: " <<
                        endpoint.address().to_string().c_str() << ":" << endpoint.port() << " previous error [" << error.message().c_str() << "]");
                _socket.async_connect(endpoint, boost::bind(&TcpInitiator<Logger, BufferPoolType, SignalType, InlineIO>::handleConnect, this, boost::asio::placeholders::error));
            }
            else
            {
                _lastEndPointIter = _endPointIter;
                VF_LOG_WARN(_logger, "Failed to connect to endpoint: " << oldendpoint.address().to_string().c_str() << ":" << oldendpoint.port() << " with error [" << error.message().c_str() << "]. Reconnecting in " << _reconnectTimer.getInterval().total_seconds() << " seconds");
                disconnect();
            }
        }
    }

    ReconnectTimer<TcpInitiator, SecondsFallbackTimer>  _reconnectTimer;
    bool                                                _reconnect;

    boost::asio::ip::tcp::resolver                      _resolver;
    boost::asio::ip::tcp::resolver::iterator            _lastEndPointIter;
    boost::asio::ip::tcp::resolver::iterator            _endPointIter;

    bool                                                _connected;
};

}  // namespace vf_common


#endif /* TCPINITIATOR_H_ */
