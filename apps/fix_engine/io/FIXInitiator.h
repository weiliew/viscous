/*
 * FIXInitiator.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef FIXINITIATOR_H_
#define FIXINITIATOR_H_

#include <sys/syscall.h>
#include <stdio.h>
#include "io/TcpInitiator.h"

using namespace vf_common;

namespace vf_fix
{

// FIXInitiator - a TCP initiator class that supports multiple end point configuration
// It alternates the connect end point in a round robin manner and retries until it runs out of
// end point to try to connect.

template<typename Logger, typename BufferPoolType, typename SignalType, typename InlineIO = std::false_type>
class FIXInitiator : public TcpInitiator<Logger, BufferPoolType, SignalType, InlineIO>
{
public:
    typedef TcpInitiator<Logger, BufferPoolType, SignalType, InlineIO>  BaseType;
    typedef BufferPoolType                                              PoolType;

    using BaseType::ProtocolType;
    using BaseType::_logger;

    FIXInitiator(Logger& logger)
    : BaseType(logger)
    , _currPos(0)
    {
    }

    virtual ~FIXInitiator()
    {
    }

    void onConnect()
    {
        BaseType::onConnect();
    }

    void onDisconnect()
    {
        // do we have a list of backup end points ? if so, set the backup end point and trigger reconnect timer
        ++_currPos;
        if(_currPos < _endPoints.size())
        {
            BaseType::setHostPort(_endPoints[_currPos]._host, _endPoints[_currPos]._port);
        }
        else if(_endPoints.size())
        {
            _currPos = 0;
            BaseType::setHostPort(_endPoints[_currPos]._host, _endPoints[_currPos]._port);
        }
        else
        {
            // all end points have been removed - stop reconnect
            BaseType::setReconnect(false); // do not attempt to reconnect
        }
        BaseType::onDisconnect();
    }

    void addEndpoint(const std::string& host, const std::string& port)
    {
        // make sure we don't already have this end point
        if(std::find(_endPoints.begin(), _endPoints.end(), EndPointType(host, port)) != _endPoints.end())
        {
            // exist
            return;
        }

        _endPoints.emplace_back(host, port);

        // set the host port on base class
        if(!_endPoints.empty())
        {
            if(_currPos >= _endPoints.size())
            {
                _currPos = 0;
            }
            BaseType::setHostPort(_endPoints[_currPos]._host, _endPoints[_currPos]._port);
        }
    }

    void removeEndpoint(const std::string& host, const std::string& port)
    {
        std::remove_if(_endPoints.begin(), _endPoints.end(), [&host, &port](const EndPointType& endpoint) -> bool{
            return host == endpoint._host && port == endpoint._port;
        });

        // once we removed an endpoint, re-pont the current endpoint position to 0
        _currPos = 0;

        // set the host port as the first in the vector
        if(!_endPoints.empty())
        {
            BaseType::setHostPort(_endPoints[_currPos]._host, _endPoints[_currPos]._port);
        }
    }


private:
    struct EndPointType
    {
        EndPointType(const std::string& host, const std::string& port)
        : _host(host)
        , _port(port)
        {
        }

        bool operator==(const EndPointType& other)
        {
            return _host == other._host && _port == other._port;
        }

        std::string _host;
        std::string _port;
    };

    size_t                      _currPos;
    std::vector<EndPointType>   _endPoints;
};

}  // namespace osf_fix


#endif /* FIXINITIATOR_H_ */
