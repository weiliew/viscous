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

template<typename Logger, typename BufferPoolType, typename SignalType, typename ProtocolType, typename InlineIO = std::true_type>
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
        if(UNLIKELY(!_socket.is_open()))
        {
            return;
        }

        _lastEndpoint = endpoint;
        asyncRead<ProtocolType>();
    }

    template<bool UseStrand = false>
    /*virtual*/ void asyncWrite(const std::string& message)
    {
        if(UNLIKELY(!_socket.is_open()))
        {
            return;
        }

        asyncWrite<ProtocolType, UseStrand>(message);
    }

    template<bool UseStrand = false>
    /*virtual*/ void asyncWrite(typename BufferPoolType::BufferPtrType buffer)
    {
        if(UNLIKELY(!_socket.is_open()))
        {
            return;
        }

        asyncWrite<ProtocolType, UseStrand>(buffer);
    }

    /*virtual*/ int syncWrite(const std::string& message)
    {
        int transferred = 0;

        if(UNLIKELY(!_socket.is_open()))
        {
            return transferred;
        }

        // check socket is alive
        int bytes = 0;
        int ret = ioctl(_socket.native(), FIONREAD, &bytes);
        boost::system::error_code error;

        // TODO - how to flush ??
        //transferred = boost::asio::write(_socket, boost::asio::buffer(message.data(), message.size()), boost::asio::transfer_all(), error);
        transferred = syncWrite<ProtocolType>(message, error);

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

    /*virtual*/ int syncWrite(typename BufferPoolType::BufferPtrType buffer)
    {
        int transferred = 0;

        if(UNLIKELY(!_socket.is_open()))
        {
            return transferred;
        }

        // check socket is alive
        int bytes = 0;
        int ret = ioctl(_socket.native(), FIONREAD, &bytes);
        boost::system::error_code error;

        // TODO - how to flush ??
        //transferred = boost::asio::write(_socket, boost::asio::buffer(buffer->buffer(), buffer->size()), boost::asio::transfer_all(), error);
        transferred = syncWrite<ProtocolType>(buffer, error);

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

        // destroy the buffer
        _bufferFactory.destroy(buffer);

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

    typename BufferPoolType::PooledFactoryType& getBufferFactory()
    {
        return _bufferFactory;
    }

protected:
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

    /*virtual*/ void handleRead(const boost::system::error_code& error, size_t bytes_transferred, typename BufferPoolType::BufferPtrType buffer)
    {
        if (LIKELY(!error))
        {
            buffer->setSize(bytes_transferred);
            processBuffer<InlineIO>(buffer);
            asyncRead<ProtocolType>();
        }
        else
        {
            VF_LOG_WARN(_logger, "Error reading data from endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                        << " with error [" << error.message().c_str() << "]. Attempting to disconnect.");
            disconnect();
        }

        // destroy the buffer
        _bufferFactory.destroy(buffer);
    }

    /*virtual*/ void handleWrite(typename BufferPoolType::BufferPtrType buffer, const boost::system::error_code& error)
    {
        if (error)
        {
            VF_LOG_WARN(_logger, "Error writing data to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                    << " with error [" << error.message().c_str() << "]. Attempting to disconnect.");
            disconnect();
        }

        // destroy the buffer
        _bufferFactory.destroy(buffer);
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

    boost::asio::io_service                         _io;
    boost::asio::io_service::work                   _endlessWork;
    boost::asio::io_service::strand                 _writeStrand;
    Logger&                                         _logger;
    typename BufferPoolType::PooledFactoryType      _bufferFactory;
    SignalType                                      _callbackSignal;
    typename ProtocolType::socket                   _socket;
    typename ProtocolType::endpoint                 _lastEndpoint;

    // callbacks
    boost::function<void (void)>                    _connectCallback;
    boost::function<void (void)>                    _disconnectCallback;

private:
    template<typename T> // if T(InlineIO) is true
    typename std::enable_if<T::value, void>::type
    processBuffer(typename BufferPoolType::BufferPtrType buffer)
    {
        _callbackSignal.dispatch(buffer);
    }

    template<typename T> // if T(InlineIO) is false
    typename std::enable_if<!T::value, void>::type
    processBuffer(typename BufferPoolType::BufferPtrType buffer)
    {
        _callbackSignal.post(buffer);
    }

    template<typename T> // if T is udp
    typename std::enable_if<std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncRead()
    {
        if(LIKELY(_socket.is_open()))
        {
            // get a new buffer from the factory
            auto buffer = _bufferFactory.create();

            _socket.async_receive_from(boost::asio::buffer(buffer->buffer(), buffer->capacity()), _lastEndpoint,
                    boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleRead,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred,
                                buffer));
        }
    }

    template<typename T> // if T is not udp
    typename std::enable_if<!std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncRead()
    {
        if(LIKELY(_socket.is_open()))
        {
            // get a new buffer from the factory
            auto buffer = _bufferFactory.create();

            _socket.async_receive(boost::asio::buffer(buffer->buffer(), buffer->capacity()),
                    boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleRead,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred,
                                buffer));
        }
    }

    template<typename T, bool UseStrand> // UseStrand and UDP
    typename std::enable_if<UseStrand && std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWrite(typename BufferPoolType::BufferPtrType buffer)
    {
        _socket.async_send_to(boost::asio::buffer(buffer->buffer(), buffer->size()), _lastEndpoint,
                _writeStrand.wrap(boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleWrite, this, buffer, boost::asio::placeholders::error)));
    }

    template<typename T, bool UseStrand> // UseStrand Not UDP
    typename std::enable_if<UseStrand && !std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWrite(typename BufferPoolType::BufferPtrType buffer)
    {
        _socket.async_send(boost::asio::buffer(buffer->buffer(), buffer->size()),
                _writeStrand.wrap(boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleWrite, this, buffer, boost::asio::placeholders::error)));
    }

    template<typename T, bool UseStrand> // Not UseStrand and UDP
    typename std::enable_if<!UseStrand && std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWrite(typename BufferPoolType::BufferPtrType buffer)
    {
        _socket.async_send_to(boost::asio::buffer(buffer->buffer(), buffer->size()), _lastEndpoint,
                boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleWrite, this, buffer, boost::asio::placeholders::error));
    }

    template<typename T, bool UseStrand> // Not UseStrand and Not UDP
    typename std::enable_if<!UseStrand && !std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWrite(typename BufferPoolType::BufferPtrType buffer)
    {
        _socket.async_send(boost::asio::buffer(buffer->buffer(), buffer->size()),
                boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleWrite, this, buffer, boost::asio::placeholders::error));
    }

    template<typename T, bool UseStrand> // UseStrand and UDP
    typename std::enable_if<UseStrand && std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWrite(const std::string& message)
    {
        _socket.async_send_to(boost::asio::buffer(message.data(), message.size()), _lastEndpoint,
                _writeStrand.wrap(boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleWrite, this, boost::asio::placeholders::error)));
    }

    template<typename T, bool UseStrand> // UseStrand and Not UDP
    typename std::enable_if<UseStrand && !std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWrite(const std::string& message)
    {
        _socket.async_send(boost::asio::buffer(message.data(), message.size()),
                _writeStrand.wrap(boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleWrite, this, boost::asio::placeholders::error)));
    }

    template<typename T, bool UseStrand> // Not UseStrand and UDP
    typename std::enable_if<!UseStrand && std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWrite(const std::string& message)
    {
        _socket.async_send_to(boost::asio::buffer(message.data(), message.size()), _lastEndpoint,
                boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleWrite, this, boost::asio::placeholders::error));
    }

    template<typename T, bool UseStrand> // Not UseStrand and Not UDP
    typename std::enable_if<!UseStrand && !std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWrite(const std::string& message)
    {
        _socket.async_send(boost::asio::buffer(message.data(), message.size()),
                boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO>::handleWrite, this, boost::asio::placeholders::error));
    }

    template<typename T> // sync op ignore UseStrand and UDP true
    typename std::enable_if<std::is_same<T, boost::asio::ip::udp>::value, size_t>::type
    syncWrite(typename BufferPoolType::BufferPtrType buffer, boost::system::error_code& error)
    {
        return _socket.send_to(boost::asio::buffer(buffer->buffer(), buffer->size()), _lastEndpoint, 0, error);
    }

    template<typename T> // sync op ignore UseStrand and UDP false
    typename std::enable_if<!std::is_same<T, boost::asio::ip::udp>::value, size_t>::type
    syncWrite(typename BufferPoolType::BufferPtrType buffer, boost::system::error_code& error)
    {
        return boost::asio::write(_socket, boost::asio::buffer(buffer->buffer(), buffer->size()), boost::asio::transfer_all(), error);
    }

    template<typename T> // sync op ignore UseStrand and UDP true
    typename std::enable_if<std::is_same<T, boost::asio::ip::udp>::value, size_t>::type
    syncWrite(const std::string& message, boost::system::error_code& error)
    {
        return _socket.send_to(boost::asio::buffer(message.data(), message.size()), _lastEndpoint, 0, error);
    }

    template<typename T> // sync op ignore UseStrand and UDP false
    typename std::enable_if<!std::is_same<T, boost::asio::ip::udp>::value, size_t>::type
    syncWrite(const std::string& message, boost::system::error_code& error)
    {
        return boost::asio::write(_socket, boost::asio::buffer(message.data(), message.size()), boost::asio::transfer_all(), error);
    }
};

}  // namespace vf_common

#endif /* IO_H_ */
