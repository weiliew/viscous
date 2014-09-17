/*
 * Signal.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef SIGNAL_H_
#define SIGNAL_H_

#include "SignalBase.h"
#include "Subscription.h"

// TODO - Replace Signal and StaticSignal with co-routine implementation for flexibility and speed

namespace vf_common
{

// TODO - might not need the PayloadType template - see StaticSignal

template<typename PayloadType>
class Signal : public SignalBase
{
public:
    struct Traits
    {
        typedef std::true_type DynamicSignal;
    };

    typedef PayloadType element_type;

    Signal(const char * name, boost::asio::io_service& ioService)
    : SignalBase(name, ioService)
    {

    }

    virtual ~Signal()
    {
    }

    // runSeq - the sequence of the call that will be made
    // runId  - subscribers with the same id will be guaranteed that the callback will be executed in a single thread if more than one thread
    //          is created for this signal
    template<typename SubscriberType>
    void subscribe(SubscriberType * subscriber, size_t runSeq = 100, size_t runId = 0)
    {
        if(!subscriber)
        {
            return;
        }

        _subscriptionList.push_back(Subscription<PayloadType>(
                subscriber,
                boost::bind(static_cast<void (SubscriberType::*)(PayloadType&)>(&SubscriberType::onData), subscriber, _1),
                runSeq,
                getStrand(runId)));

        std::sort(_subscriptionList.begin(), _subscriptionList.end());
    }

    template<typename SubscriberType>
    void unsubscribe(SubscriberType * subscriber)
    {
        if(!subscriber)
        {
            return;
        }

        typename std::vector<Subscription<PayloadType> >::iterator iter = _subscriptionList.begin();
        while(iter != _subscriptionList.end())
        {
            if((*iter)._subscriber == subscriber)
            {
                typename std::vector<Subscription<PayloadType> >::iterator toDel = iter;
                ++iter;
                _subscriptionList.erase(toDel);
                continue;
            }
            ++iter;
        }
    }

    // async post
    inline void post(PayloadType& payload)
    {
        for_each(_subscriptionList.begin(), _subscriptionList.end(), [&payload](Subscription<PayloadType>& subscription){
            // is using lamda faster than boost::bind here ??
            //subscriber._strand.post( boost::bind( &Subscription<PayloadType>::postWork, &(*iter), payload) );
            subscription._strand->post( [&subscription, payload](){
                subscription.postWork(payload);
            });
        });
    }

    // inline dispatch on the current thread
    inline void dispatch(PayloadType& payload)
    {
        for_each(_subscriptionList.begin(), _subscriptionList.end(), [&payload](Subscription<PayloadType>& subscription){
            subscription._fn(payload);
        });
    }

private:
    std::vector<Subscription<PayloadType> > _subscriptionList;
};


template<typename PayloadType>
class Signal<PayloadType *> : public SignalBase
{
public:
    struct Traits
    {
        typedef std::true_type DynamicSignal;
    };

    typedef PayloadType *  element_type;

    Signal(const char * name, boost::asio::io_service& ioService)
    : SignalBase(name, ioService)
    {

    }

    virtual ~Signal()
    {
    }

    // runSeq - the sequence of the call that will be made
    // runId  - subscribers with the same id will be guaranteed that the callback will be executed in a single thread if more than one thread
    //          is created for this signal
    template<typename SubscriberType>
    void subscribe(SubscriberType * subscriber, size_t runSeq = 100, size_t runId = 0)
    {
        if(!subscriber)
        {
            return;
        }

        _subscriptionList.push_back(Subscription<PayloadType*>(
                subscriber,
                boost::bind(static_cast<void (SubscriberType::*)(PayloadType*&)>(&SubscriberType::onData), subscriber, _1),
                runSeq,
                getStrand(runId)));

        std::sort(_subscriptionList.begin(), _subscriptionList.end());
    }

    template<typename SubscriberType>
    void unsubscribe(SubscriberType * subscriber)
    {
        if(!subscriber)
        {
            return;
        }

        typename std::vector<Subscription<PayloadType*> >::iterator iter = _subscriptionList.begin();
        while(iter != _subscriptionList.end())
        {
            if((*iter)._subscriber == subscriber)
            {
                typename std::vector<Subscription<PayloadType*> >::iterator toDel = iter;
                ++iter;
                _subscriptionList.erase(toDel);
                continue;
            }
            ++iter;
        }
    }

    // async post
    inline void post(PayloadType* payload)
    {
        for_each(_subscriptionList.begin(), _subscriptionList.end(), [payload](Subscription<PayloadType*>& subscription){
            // is using lamda faster than boost::bind here ??
            //subscriber._strand.post( boost::bind( &Subscription<PayloadType>::postWork, &(*iter), payload) );
            subscription._strand->post( [&subscription, payload](){
                subscription.postWork(payload);
            });
        });
    }

    // inline dispatch on the current thread
    inline void dispatch(PayloadType* payload)
    {
        for_each(_subscriptionList.begin(), _subscriptionList.end(), [payload](Subscription<PayloadType*>& subscription){
            subscription._fn(payload);
        });
    }

private:
    std::vector<Subscription<PayloadType*> > _subscriptionList;
};

}  // namespace vf_common


#endif /* SIGNAL_H_ */
