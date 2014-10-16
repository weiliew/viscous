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
#include <functional>

#include "utilities/Utilities.h"
#include "logging/Log.h"

// TODO - Create new version using co-routine version of asio and test performance against this

namespace vf_common
{

template<typename Logger, typename BufferPoolType, typename SignalType,
         typename ProtocolType, typename InlineIO, typename DirectDataCallback = std::false_type>
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
    void asyncWrite(const std::string& message)
    {
        if(UNLIKELY(!_socket.is_open()))
        {
            return;
        }

        asyncWriteBuffer<ProtocolType, UseStrand>(boost::asio::const_buffer(message.data(), message.length()));
    }

    // TODO - sending and receiving of iovec does not work ccurrently
    template<bool UseStrand = false>
    void asyncWrite(const iovec* vec, size_t count)
    {
        if(UNLIKELY(!_socket.is_open()))
        {
            return;
        }

        // TODO - must make this more efficient - maybe call writev directly ?
        std::vector<boost::asio::const_buffer> bvec;
        for(size_t i=0;i<count;++i)
        {
            bvec.push_back(boost::asio::const_buffer(vec[i].iov_base, vec[i].iov_len));
        }
        asyncWriteBuffer<ProtocolType, UseStrand>(bvec);
    }

    template<bool UseStrand = false>
    void asyncWrite(typename BufferPoolType::BufferPtrType buffer)
    {
        if(UNLIKELY(!_socket.is_open()))
        {
            return;
        }

        asyncWriteBuffer<ProtocolType, UseStrand>(boost::asio::const_buffer(buffer->buffer(), buffer->size()));
    }

    template<bool UseStrand = false>
    void asyncWrite(boost::asio::const_buffer& buffer)
    {
        if(UNLIKELY(!_socket.is_open()))
        {
            return;
        }

        asyncWriteBuffer<ProtocolType, UseStrand>(buffer);
    }

    size_t syncWrite(const std::string& message)
    {
        return syncWriteBuffer(boost::asio::const_buffer(message.data(), message.length()));
    }

    // TODO - sending and receiving of iovec does not work ccurrently
    size_t syncWrite(const iovec* vec, size_t count)
    {
        // TODO - must make this more efficient - maybe call writev directly ?
        std::vector<boost::asio::const_buffer> bvec;
        for(size_t i=0;i<count;++i)
        {
            bvec.push_back(boost::asio::const_buffer(vec[i].iov_base, vec[i].iov_len));
        }
        return syncWriteBuffer(bvec);
    }

    size_t syncWrite(typename BufferPoolType::BufferPtrType buffer)
    {
        return syncWriteBuffer(boost::asio::const_buffer(buffer->buffer(), buffer->size()));
    }

    size_t syncWrite(boost::asio::const_buffer& buffer)
    {
        return syncWriteBuffer(buffer);
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

    void registerConnectCallback(std::function<void (void)> func)
    {
        _connectCallback = func;
    }

    void registerDisconnectCallback(std::function<void (void)> func)
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

    void registerDirectCallback(std::function<void (typename BufferPoolType::BufferPtrType buffer)> func)
    {
        _dataCallback = func;
    }

    const Logger& logger()
    {
        return _logger;
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
            processBuffer<InlineIO, DirectDataCallback>(buffer);
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

    template<typename BufferType>
    void handleWrite(const BufferType& buffer, const boost::system::error_code& error)
    {
        if (error)
        {
            VF_LOG_WARN(_logger, "Error writing data to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":" << _lastEndpoint.port()
                    << " with error [" << error.message().c_str() << "]. Attempting to disconnect.");
            disconnect();
        }
    }

    void handleWrite(const boost::system::error_code& error)
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
    std::function<void (void)>                      _connectCallback;
    std::function<void (void)>                      _disconnectCallback;

    std::function<void (typename BufferPoolType::BufferPtrType buffer)> _dataCallback;

private:
    template<typename T, typename D> // if T(InlineIO) is true and not direct (D)
    typename std::enable_if<T::value && !D::value, void>::type
    processBuffer(typename BufferPoolType::BufferPtrType buffer)
    {
        _callbackSignal.dispatch(buffer);
    }

    template<typename T, typename D> // if T(InlineIO) is false and not direct (D)
    typename std::enable_if<!T::value && !D::value, void>::type
    processBuffer(typename BufferPoolType::BufferPtrType buffer)
    {
        _callbackSignal.post(buffer);
    }

    template<typename T, typename D> // if DirectDataCallback is true
    typename std::enable_if<T::value && D::value, void>::type
    processBuffer(typename BufferPoolType::BufferPtrType buffer)
    {
        if(LIKELY(_dataCallback))
        {
            _dataCallback(buffer);
        }
    }

    template<typename T, typename D> // // if DirectDataCallback is true
    typename std::enable_if<!T::value && D::value, void>::type
    processBuffer(typename BufferPoolType::BufferPtrType buffer)
    {
        if(LIKELY(_dataCallback))
        {
            _dataCallback(buffer);
        }
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
                    boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO, DirectDataCallback>::handleRead,
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
                    boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO, DirectDataCallback>::handleRead,
                                this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred,
                                buffer));
        }
    }

    template<typename T, bool UseStrand, typename BufferType> // UseStrand and UDP
    typename std::enable_if<UseStrand && std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWriteBuffer(const BufferType& buffer)
    {
        _socket.async_send_to(boost::asio::buffer(buffer), _lastEndpoint,
                _writeStrand.wrap(boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO, DirectDataCallback>::handleWrite<BufferType>, this, buffer, boost::asio::placeholders::error)));
    }

    template<typename T, bool UseStrand, typename BufferType> // UseStrand Not UDP
    typename std::enable_if<UseStrand && !std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWriteBuffer(const BufferType& buffer)
    {
        _socket.async_send(boost::asio::buffer(buffer),
                _writeStrand.wrap(boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO, DirectDataCallback>::handleWrite<BufferType>, this, buffer, boost::asio::placeholders::error)));
    }

    template<typename T, bool UseStrand, typename BufferType> // Not UseStrand and UDP
    typename std::enable_if<!UseStrand && std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWriteBuffer(const BufferType& buffer)
    {
        _socket.async_send_to(boost::asio::buffer(buffer), _lastEndpoint,
                boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO, DirectDataCallback>::handleWrite<BufferType>, this, buffer, boost::asio::placeholders::error));
    }

    template<typename T, bool UseStrand, typename BufferType> // Not UseStrand and Not UDP
    typename std::enable_if<!UseStrand && !std::is_same<T, boost::asio::ip::udp>::value, void>::type
    asyncWriteBuffer(const BufferType& buffer)
    {
        _socket.async_send(boost::asio::buffer(buffer),
                boost::bind(&IO<Logger, BufferPoolType, SignalType, ProtocolType, InlineIO, DirectDataCallback>::handleWrite<BufferType>, this, buffer, boost::asio::placeholders::error));
    }

    template<typename BufferType>
    size_t syncWriteBuffer(const BufferType& buffer)
    {
        size_t transferred = 0;

        if(UNLIKELY(!_socket.is_open()))
        {
            return transferred;
        }

        // check socket is alive
        int bytes = 0;
        int ret = ioctl(_socket.native(), FIONREAD, &bytes);
        boost::system::error_code error;

        // TODO - how to flush ??
        //transferred = boost::asio::write(_socket, buffer, boost::asio::transfer_all(), error);
        transferred = syncWrite<ProtocolType>(buffer, error);

        if (transferred < boost::asio::buffer_size(buffer) || error)
        {
            if(error)
            {
                VF_LOG_WARN(_logger, "Failed to write to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":"
                    << _lastEndpoint.port() << " with error [" << error.message().c_str() << "]");
            }
            else
            {
                VF_LOG_WARN(_logger, "Failed to write to endpoint: " << _lastEndpoint.address().to_string().c_str() << ":"
                    << _lastEndpoint.port() << " with no error");
            }
        }

        return transferred;
    }

    template<typename T, typename BufferType> // sync op ignore UseStrand and UDP true
    typename std::enable_if<std::is_same<T, boost::asio::ip::udp>::value, size_t>::type
    syncWrite(const BufferType& buffer, boost::system::error_code& error)
    {
        return _socket.send_to(boost::asio::buffer(buffer), _lastEndpoint, 0, error);
    }

    template<typename T, typename BufferType> // sync op ignore UseStrand and UDP false
    typename std::enable_if<!std::is_same<T, boost::asio::ip::udp>::value, size_t>::type
    syncWrite(const BufferType& buffer, boost::system::error_code& error)
    {
        return boost::asio::write(_socket, boost::asio::buffer(buffer), boost::asio::transfer_all(), error);
    }
};

}  // namespace vf_common

#endif /* IO_H_ */
