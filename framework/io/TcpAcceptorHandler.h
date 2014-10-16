/*
 * TcpAcceptorHandler.h
 *
 *  Created on: 2 Mar 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef TCPACCEPTORHANDLER_H_
#define TCPACCEPTORHANDLER_H_

#include "timer/TimerDefs.h"

namespace vf_common
{

template<typename AcceptorType>
class TcpAcceptorHandler : public SecondsTimer
{
public:
    typedef TcpAcceptorHandler<AcceptorType>        HandlerType;
    typedef std::shared_ptr<AcceptorType>           AcceptorPtrType;
    typedef typename AcceptorType::LoggerT          Logger;
    typedef typename AcceptorType::BufferPoolTypeT  BufferPoolType;
    typedef typename AcceptorType::SignalTypeT      SignalType;
    typedef typename AcceptorType::InlineIOT        InlineIO;

    TcpAcceptorHandler(boost::asio::io_service& io, Logger& logger)
    : SecondsTimer(io)
    , _logger(logger)
    , _acceptor(std::make_shared<boost::asio::ip::tcp::acceptor>(io))
    , _ioService(io)
    , _newAcceptorSignal("NewAcceptorSignal", io)
    {
        // TODO - make configurable ?
        SecondsTimer::setInterval(5); // clean up every 5 seconds
    }

    /*virtual*/ ~TcpAcceptorHandler()
    {
    }

    bool start(const std::string& host, const std::string& port)
    {
        SecondsTimer::runTimer();
        VF_LOG_INFO(_logger, "Initialising acceptor for host:port = " << host << ":" << port);

        try
        {
            // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
            boost::asio::ip::tcp::resolver resolver(_ioService);
            boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(boost::asio::ip::tcp::resolver::query(host, port));

            _acceptor->open(endpoint.protocol());
            _acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            _acceptor->bind(endpoint);
            _acceptor->listen();
            accept();
        }
        catch (boost::system::system_error& e)
        {
            VF_LOG_WARN(_logger, "Failed to initialise acceptor host:port = " << host << ":" << port << " [" << e.what() << "]");
            return false;
        }

        return true;
    }

    Signal<AcceptorPtrType>& newAcceptorSignal()
    {
        return _newAcceptorSignal;
    }

    std::shared_ptr<boost::asio::ip::tcp::acceptor> getAcceptor()
    {
        return _acceptor;
    }

    void removeAcceptor(boost::asio::ip::tcp::endpoint& endpoint)
    {
        boost::mutex::scoped_lock lock(_handlerMutex);
        auto findIter = _tcpAcceptorMap.find(endpoint);
        if(findIter != _tcpAcceptorMap.end())
        {
            _tcpAcceptorCleanUpList.push_back((*findIter).second);
            _tcpAcceptorMap.erase(findIter);
        }
    }

private:
    bool onTimer()
    {
        boost::mutex::scoped_lock lock(_handlerMutex);
        _tcpAcceptorCleanUpList.clear();
        return false;
    }

    void handleAccept(const boost::system::error_code& error, AcceptorPtrType acceptor)
    {
        boost::asio::ip::tcp::endpoint endpoint = acceptor->getSocket().remote_endpoint();
        if (LIKELY(!error))
        {
            // listen for any new client first
            accept();

            // check if we already have an acceptor for this client, if so, disconnect the new attempt
            {
                boost::mutex::scoped_lock lock(_handlerMutex);
                if(_tcpAcceptorMap.find(endpoint)!= _tcpAcceptorMap.end())
                {
                    VF_LOG_INFO(_logger, "Client already connected previously on "
                            << endpoint.address().to_string().c_str() << ":" << endpoint.port() << ". Disconnecting client.");
                    return;
                }
            }

            _tcpAcceptorMap.insert(std::make_pair(endpoint, acceptor));
            acceptor->setDisconnectCallback(std::bind(&HandlerType::removeAcceptor, this, std::placeholders::_1));

            _newAcceptorSignal.dispatch(acceptor);
            acceptor->onAccept(endpoint);
        }
        else
        {
            VF_LOG_WARN(_logger, "Failed to accept from endpoint: " << endpoint.address().to_string().c_str() << ":" << endpoint.port() << " with error [" << error.message().c_str() << "]");
        }
    }

    void accept()
    {
        boost::asio::ip::tcp::endpoint endpoint = _acceptor->local_endpoint();
        VF_LOG_INFO(_logger, "Listening for incoming client connect on " << endpoint.address().to_string().c_str() << ":" << endpoint.port());

        // create a new acceptor
        auto newAcceptor = std::make_shared<AcceptorType>(_logger);
        _acceptor->async_accept(newAcceptor->getSocket(), endpoint, boost::bind(&HandlerType::handleAccept,
                this, boost::asio::placeholders::error, newAcceptor));
    }

    Logger                                                       _logger;
    std::shared_ptr<boost::asio::ip::tcp::acceptor>              _acceptor;
    boost::asio::io_service&                                     _ioService;
    Signal<AcceptorPtrType>                                      _newAcceptorSignal;
    boost::mutex                                                 _handlerMutex;
    std::map<boost::asio::ip::tcp::endpoint, AcceptorPtrType>    _tcpAcceptorMap;
    std::vector<AcceptorPtrType>                                 _tcpAcceptorCleanUpList;
};


}  // namespace vf_common



#endif /* TCPACCEPTORHANDLER_H_ */
