/*
 * UdpPublisher.h
 *
 *  Created on: 3 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef UDPPUBLISHER_H_
#define UDPPUBLISHER_H_

#include "IO.h"
#include "signals/Signal.h"

namespace vf_common
{

template<typename Logger, typename BufferPoolType, typename SignalType, typename InlineIO = std::true_type>
class UdpPublisher : public IO<Logger, BufferPoolType, SignalType, boost::asio::ip::udp, InlineIO>
{
public:
    typedef IO<Logger, BufferPoolType, SignalType, boost::asio::ip::udp, InlineIO> BaseType;

    using BaseType::_logger;
    using BaseType::_socket;
    using BaseType::_lastEndpoint;

    UdpPublisher(Logger& logger)
    : BaseType(logger)
    , _interface("")
    , _connected(false)
    {
    }

    virtual ~UdpPublisher()
    {
    }

    bool connect()
    {
        if(_connected)
        {
            return true;
        }

        try
        {
            _socket.open(_lastEndpoint.protocol());

            if(!_interface.empty())
            {
                _socket.set_option(boost::asio::ip::multicast::outbound_interface(boost::asio::ip::address::from_string(_interface).to_v4()));
            }

            onConnect();
            return true;
        }
        catch(boost::system::system_error& e)
        {
            VF_LOG_WARN(_logger, "Failed to start UDP publisher host:port:interface = "
                    << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port() << ":" << (_interface.empty() ? "any" : _interface) << " [" << e.what() << "]");
        }

        return false;
    }

    void disconnect(bool reconnect = true)
    {
        _socket.close();
        onDisconnect();
    }

    virtual void onConnect()
    {
        // TODO - config
        _socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        _socket.set_option(boost::asio::ip::multicast::enable_loopback(true));

        _socket.set_option(boost::asio::socket_base::receive_buffer_size(16777216)); // 16MB buffer size
        _socket.set_option(boost::asio::socket_base::send_buffer_size(16777216)); // 16MB buffer size

        BaseType::onConnect();
        _connected = true;
    }

    virtual void onDisconnect()
    {
        BaseType::onDisconnect();
        _connected = false;
    }

    bool start(const std::string& addr, const std::string& port, const std::string& interface = "")
    {
        _interface = interface;
        try
        {
            VF_LOG_INFO(_logger, "Starting UDP publisher for host:port:interface = "
                    << addr << ":" << port << ":" << (interface.empty() ? "any" : interface));

            _lastEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(addr), atoi(port.c_str()));
            return connect();
        }
        catch (boost::system::system_error& e)
        {
            VF_LOG_WARN(_logger, "Failed to initialise UDP publisher host:port:interface = "
                    << addr << ":" << port << ":" << (interface.empty() ? "any" : interface) << " [" << e.what() << "]");
        }

        return false;
    }

    bool connected()
    {
        return _connected;
    }

private:

    std::string         _interface;
    bool                _connected;
};

} // namespace vf_common

#endif /* UDPPUBLISHER_H_ */
