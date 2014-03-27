/*
 * IO.h
 *
 *  Created on: 3 Feb 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef IO_H_
#define IO_H_

#include <type_traits>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "utilities/Utilities.h"
#include "logging/Log.h"

// TODO - Create new version using co-routine version of asio and test performance against this

namespace vf_common
{

template<typename Logger, typename BufferFactoryType, typename SignalType, typename ProtocolType, typename InlineIO = std::true_type>
class IO
{
public:
    IO(Logger& logger)
    : _endlessWork      (_io)
    , _writeStrand      (_io)
    , _logger           (logger)
    , _callbackSignal   ("IOCallbackSignal", _io)
    , _socket           (_io)
    {
    }

    virtual ~IO()
    {
    }

    IO(const IO& copy) = delete;
    IO() = delete;

    /*virtual*/ void asyncRead(typename ProtocolType::endpoint endpoint)
    {
        if(unlikely(!_socket.is_open()))
        {
            return;
        }

        _lastEndpoint = endpoint;
        asyncRead();
    }


    template<typename UseStrand = std::true_type>
    void asyncWrite(const std::string& message)
    {
        asyncWrite(message, UseStrand());
    }

    template<typename UseStrand = std::true_type>
    void asyncWrite(std::shared_ptr<typename BufferFactoryType::ElementType> buffer)
    {
        asyncWrite(buffer, UseStrand());
    }

    /*virtual*/ int syncWrite(const std::string& message)
    {
        int transferred = 0;

        if(unlikely(!_socket.is_open()))
        {
            return transferred;
        }

        // check socket is alive
        int bytes = 0;
        int ret = ioctl(_socket.native(), FIONREAD, &bytes);
        boost::system::error_code error;

        // TODO - how to flush ??
        transferred = boost::asio::write(_socket, boost::asio::buffer(message.data(), message.size()), boost::asio::transfer_all(), error);
        if (transferred < message.size() || error)
        {
            if(error)
            {
                VF_LOG_WARN(_logger, "Failed to write to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                    << " with error [" << error.message().c_str() << "] for message [" << message << "]");
            }
            else
            {
                VF_LOG_WARN(_logger, "Failed to write to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                    << " with no error for message [" << message << "]");
            }
        }

        return transferred;
    }

    /*virtual*/ int syncWrite(std::shared_ptr<typename BufferFactoryType::ElementType> buffer)
    {
        int transferred = 0;

        if(unlikely(!_socket.is_open()))
        {
            return transferred;
        }

        // check socket is alive
        int bytes = 0;
        int ret = ioctl(_socket.native(), FIONREAD, &bytes);
        boost::system::error_code error;

        // TODO - how to flush ??
        transferred = boost::asio::write(_socket, boost::asio::buffer(buffer->buffer(), buffer->size()), boost::asio::transfer_all(), error);
        if (transferred < buffer->size() || error)
        {
            if(error)
            {
                VF_LOG_WARN(_logger, "Failed to write to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                    << " with error [" << error.message().c_str() << "] for message [" << std::string(buffer->buffer(), buffer->size()).c_str() << "]");
            }
            else
            {
                VF_LOG_WARN(_logger, "Failed to write to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                    << " with no error for message [" << std::string(buffer->buffer(), buffer->size()).c_str() << "]");
            }
        }

        return transferred;
    }

    virtual void disconnect(bool reconnect = true) = 0;

    virtual void onDisconnect()
    {
        VF_LOG_INFO(_logger, "Disconnected endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port());
        if(_disconnectCallback)
        {
            _disconnectCallback();
        }
    }

    virtual void onConnect()
    {
        VF_LOG_INFO(_logger, "Connected endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port());

        if(_connectCallback)
        {
            _connectCallback();
        }
    }

    typename ProtocolType::socket& getSocket()
    {
        return _socket;
    }

    SignalType& getCallbackSignal()
    {
        return _callbackSignal;
    }

    void registerConnectCallback(boost::function<void (void)> func)
    {
        _connectCallback = func;
    }

    void registerDisconnectCallback(boost::function<void (void)> func)
    {
        _disconnectCallback = func;
    }

    boost::asio::io_service& getIO()
    {
        return _io;
    }

protected:

    // TODO - decide if we are doing message buffer segmentation here or later ?? ??
    /*virtual*/ void handleRead(const boost::system::error_code& error, size_t bytes_transferred, std::shared_ptr<typename BufferFactoryType::ElementType> buffer)
    {
        if (likely(!error))
        {
            buffer->setSize(bytes_transferred);
            processBuffer(buffer, InlineIO());
            asyncRead();
        }
        else
        {
            VF_LOG_WARN(_logger, "Error reading data from endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                        << " with error [" << error.message().c_str() << "]. Attempting to disconnect.");
            disconnect();
        }
    }

    /*virtual*/ void asyncWrite(std::shared_ptr<typename BufferFactoryType::ElementType> buffer, std::true_type)
    {
        if(unlikely(!_socket.is_open()))
        {
            return;
        }

        boost::asio::async_write(_socket,
                boost::asio::buffer(buffer->buffer(), buffer->size()),
                _writeStrand.wrap(boost::bind(&IO<Logger, BufferFactoryType, SignalType, ProtocolType, InlineIO>::handleWrite, this, boost::asio::placeholders::error)));
    }

    /*virtual*/ void asyncWrite(std::shared_ptr<typename BufferFactoryType::ElementType> buffer, std::false_type)
    {
        if(unlikely(!_socket.is_open()))
        {
            return;
        }

        boost::asio::async_write(_socket,
                boost::asio::buffer(buffer->buffer(), buffer->size()),
                boost::bind(&IO<Logger, BufferFactoryType, SignalType, ProtocolType, InlineIO>::handleWrite, this, boost::asio::placeholders::error));
    }

    /*virtual*/ void asyncWrite(const std::string& message, std::true_type)
    {
        if(unlikely(!_socket.is_open()))
        {
            return;
        }

        boost::asio::async_write(_socket,
                boost::asio::buffer(message.data(), message.size()),
                _writeStrand.wrap(boost::bind(&IO<Logger, BufferFactoryType, SignalType, ProtocolType, InlineIO>::handleWrite, this, boost::asio::placeholders::error)));
    }

    /*virtual*/ void asyncWrite(const std::string& message, std::false_type)
    {
        if(unlikely(!_socket.is_open()))
        {
            return;
        }

        boost::asio::async_write(_socket,
                boost::asio::buffer(message.data(), message.size()),
                boost::bind(&IO<Logger, BufferFactoryType, SignalType, ProtocolType, InlineIO>::handleWrite, this, boost::asio::placeholders::error));
    }

    void processBuffer(std::shared_ptr<typename BufferFactoryType::ElementType>& buffer, std::true_type)
    {
        _callbackSignal.dispatch(buffer);
    }

    /*virtual*/ void processBuffer(std::shared_ptr<typename BufferFactoryType::ElementType>& buffer, std::false_type)
    {
        _callbackSignal.post(buffer);
    }

    /*virtual*/ void handleIoError(const boost::system::error_code& error)
    {
        if(error)
        {
            // check for all possible boost asio error types
            // TODO

            VF_LOG_WARN(_logger, "Error reading data from endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                    << " with error [" << error.message().c_str() << ". Attempting to disconnect.");
            disconnect();
        }
    }

    /*virtual*/ void handleWrite(const boost::system::error_code& error)
    {
        if (error)
        {
            VF_LOG_WARN(_logger, "Error writing data to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                    << " with error [" << error.message().c_str() << "]. Attempting to disconnect.");
            disconnect();
        }
    }

    boost::asio::io_service             _io;
    boost::asio::io_service::work       _endlessWork;
    boost::asio::io_service::strand     _writeStrand;
    Logger&                             _logger;
    BufferFactoryType                   _bufferFactory;
    SignalType                          _callbackSignal;
    typename ProtocolType::socket       _socket;
    typename ProtocolType::endpoint     _lastEndpoint;

    // callbacks
    boost::function<void (void)>        _connectCallback;
    boost::function<void (void)>        _disconnectCallback;

private:
    void asyncRead()
    {
        if(unlikely(!_socket.is_open()))
        {
            return;
        }

        // get a new buffer from the factory
        std::shared_ptr<typename BufferFactoryType::ElementType> buffer(_bufferFactory.create());

        _socket.async_read_some(boost::asio::buffer(buffer->buffer(), buffer->capacity()),
                boost::bind(&IO<Logger, BufferFactoryType, SignalType, ProtocolType, InlineIO>::handleRead,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            buffer));
    }

};

}  // namespace vf_common

#endif /* IO_H_ */
