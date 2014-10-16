/*
 * TcpAcceptor.h
 *
 *  Created on: 3 Feb 2014
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef TCPACCEPTOR_H_
#define TCPACCEPTOR_H_

#include "IO.h"
#include "signals/Signal.h"
#include <functional>

namespace vf_common
{

template<typename Logger, typename BufferPoolType, typename SignalType, typename InlineIO = std::false_type>
class TcpAcceptor : public IO<Logger, BufferPoolType, SignalType, boost::asio::ip::tcp, InlineIO>
{
public:
    typedef boost::asio::ip::tcp                                            ProtocolType;
    typedef IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>  BaseType;

    typedef Logger          LoggerT;
    typedef BufferPoolType  BufferPoolTypeT;
    typedef SignalType      SignalTypeT;
    typedef InlineIO        InlineIOT;

    using BaseType::_logger;
    using BaseType::_socket;
    using BaseType::_lastEndpoint; // last endpoint of an acceptor is the peer endpoint
    using BaseType::_io;

    TcpAcceptor(Logger& logger)
    : BaseType(logger)
    , _thread(NULL)
    , _connected(false)
    {
    }

    virtual ~TcpAcceptor()
    {
        _io.stop();
        if(_thread)
        {
            _thread->join();
        }
    }

    virtual void disconnect(bool reconnect = true)
    {
        if(BaseType::_socket.is_open())
        {
            // Initiate graceful connection closure.
            BaseType::_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            BaseType::onDisconnect();
        }

        _connected = false;

        // trigger disconnect callback and we are done
        if(_disconnectCallback)
        {
            _disconnectCallback(_lastEndpoint);
        }
    }

    void setDisconnectCallback(std::function<void (boost::asio::ip::tcp::endpoint&)> cb)
    {
        _disconnectCallback = cb;
    }

    // TODO - might want to specify threads per acceptor - for now, one thread per acceptor
    void onAccept(boost::asio::ip::tcp::endpoint endpoint)
    {
        // spawn a thread to run the io service - TODO - do we need to add threads for this io ??
        _thread =  std::make_shared<std::thread>(boost::bind(&boost::asio::io_service::run, &_io));

        // TODO - config
        _socket.set_option(boost::asio::ip::tcp::no_delay(true)); // disable nagles
        _socket.set_option(boost::asio::socket_base::receive_buffer_size(16777216)); // 16MB buffer size

        _lastEndpoint = endpoint;
        _connected = true;

        BaseType::onConnect();
        BaseType::asyncRead(endpoint);
    }

    bool connected()
    {
        return _connected;
    }

private:
    std::shared_ptr<std::thread>                            _thread;
    std::function<void (boost::asio::ip::tcp::endpoint&)>   _disconnectCallback;
    bool                                                    _connected;
};

}  // namespace vf_common

#endif /* TCPACCEPTOR_H_ */
