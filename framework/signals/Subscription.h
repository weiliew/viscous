/*
 * Subscription.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef SUBSCRIPTION_H_
#define SUBSCRIPTION_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <stdio.h>

namespace vf_common
{

template<typename PayloadType>
class Subscription
{
public:
    typedef boost::function<void (PayloadType& msg)> CallbackFnType;

    Subscription(void * sub, CallbackFnType fn, size_t runSeq, std::shared_ptr<boost::asio::strand> strand)
    : _runSequence(runSeq)
    , _subscriber(sub)
    , _fn(fn)
    , _strand(strand)
    {
    }

    Subscription(const Subscription& copy)
    : _runSequence(copy._runSequence)
    , _subscriber(copy._subscriber)
    , _fn(copy._fn)
    , _strand(copy._strand)
    {
    }

    bool operator<(const Subscription rhs) const
    {
        return _runSequence < rhs._runSequence;
    }

    void postWork(PayloadType payload)
    {
        _fn(payload);
    }

    size_t                                  _runSequence;
    void *                                  _subscriber;
    CallbackFnType                          _fn;
    std::shared_ptr<boost::asio::strand>    _strand;
};

template<typename PayloadType>
class Subscription<PayloadType*>
{
public:
    typedef boost::function<void (PayloadType* msg)> CallbackFnType;

    Subscription(void * sub, CallbackFnType fn, size_t runSeq, std::shared_ptr<boost::asio::strand> strand)
    : _runSequence(runSeq)
    , _subscriber(sub)
    , _fn(fn)
    , _strand(strand)
    {
    }

    Subscription(const Subscription& copy)
    : _runSequence(copy._runSequence)
    , _subscriber(copy._subscriber)
    , _fn(copy._fn)
    , _strand(copy._strand)
    {
    }

    bool operator<(const Subscription rhs) const
    {
        return _runSequence < rhs._runSequence;
    }

    void postWork(PayloadType* payload)
    {
        _fn(payload);
    }

    size_t                                  _runSequence;
    void *                                  _subscriber;
    CallbackFnType                          _fn;
    std::shared_ptr<boost::asio::strand>    _strand;
};

}  // namespace vf_common


#endif /* SUBSCRIPTION_H_ */
