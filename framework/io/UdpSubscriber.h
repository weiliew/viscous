/*
 * UdpSubscriber.h
 *
 *  Created on: 3 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef UDPSUBSCRIBER_H_
#define UDPSUBSCRIBER_H_

#include "IO.h"
#include "signals/Signal.h"

namespace vf_common
{

template<typename Logger, typename BufferPoolType, typename SignalType, typename InlineIO = std::true_type>
class UdpSubscriber : public IO<Logger, BufferPoolType, SignalType, boost::asio::ip::udp, InlineIO>
{
public:
    typedef IO<Logger, BufferPoolType, SignalType, boost::asio::ip::udp, InlineIO> BaseType;

    using BaseType::_logger;
    using BaseType::_socket;
    using BaseType::_lastEndpoint;

    UdpSubscriber(Logger& logger)
    : BaseType(logger)
    , _connected(false)
    {
    }

    virtual ~UdpSubscriber()
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
            _socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
            _socket.set_option(boost::asio::ip::multicast::enable_loopback(true));

            //_socket.bind(_lastEndpoint);
            // workaround for binding to interface in Linux - http://thread.gmane.org/gmane.comp.lib.boost.asio.user/609
            _socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(_mcAddr), _lastEndpoint.port()));
            _socket.set_option(boost::asio::ip::multicast::join_group(boost::asio::ip::address::from_string(_mcAddr).to_v4(),
                                                                      _lastEndpoint.address().to_v4()));

            onConnect();
            return true;
        }
        catch(boost::system::system_error& e)
        {
            VF_LOG_WARN(_logger, "Failed to start UDP receiver host:port:mcgroup = "
                    << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port() << ":" << _mcAddr << " [" << e.what() << "]");
        }

        return false;
    }

    void disconnect(bool reconnect = true)
    {
        _socket.set_option(boost::asio::ip::multicast::leave_group(boost::asio::ip::address::from_string(_mcAddr)));
        _socket.close();
        onDisconnect();
    }

    virtual void onConnect()
    {
        // TODO - config
        _socket.set_option(boost::asio::socket_base::receive_buffer_size(16777216)); // 16MB buffer size
        _socket.set_option(boost::asio::socket_base::send_buffer_size(16777216)); // 16MB buffer size

        BaseType::onConnect();
        _connected = true;

        BaseType::asyncRead(_lastEndpoint);
    }

    virtual void onDisconnect()
    {
        BaseType::onDisconnect();
        _connected = false;
    }

    bool start(const std::string& addr, const std::string& port, const std::string& interface)
    {
        _mcAddr = addr;
        try
        {
            VF_LOG_INFO(_logger, "Starting UDP subscriber for host:port:interface = "
                    << addr << ":" << port << ":" << interface);

            _lastEndpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(interface), atoi(port.c_str()));
            return connect();
        }
        catch (boost::system::system_error& e)
        {
            VF_LOG_WARN(_logger, "Failed to initialise UDP subscriber host:port:interface = "
                    << addr << ":" << port << ":" << interface << " [" << e.what() << "]");
        }

        return false;
    }

    bool connected()
    {
        return _connected;
    }

private:

    std::string         _mcAddr;
    bool                _connected;

};

} // namespace vf_common

#endif /* UDPSUBSCRIBER_H_ */
