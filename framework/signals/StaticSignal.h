/*
 * StaticSignal.h
 *
 *  Created on: 17 Jan 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef STATICSIGNAL_H_
#define STATICSIGNAL_H_

#include <tuple>
#include <iostream>
#include "SignalBase.h"

namespace vf_common
{

template<typename... Subscribers>
class StaticSignal : public SignalBase
{
public:
    StaticSignal(const char * name, boost::asio::io_service& ioService, Subscribers... subscriberList)
    : SignalBase(name, ioService)
    , _subscriberList(std::make_tuple(subscriberList...))
    {
    }

    virtual ~StaticSignal()
    {
    }

    template<typename PayloadType>
    inline void post(std::shared_ptr<PayloadType>& payload)
    {
        postSeq(payload, typename gens<sizeof...(Subscribers)>::type());
    }

    template<typename PayloadType>
    void dispatch(std::shared_ptr<PayloadType>& payload)
    {
        dispatchSeq(payload, typename gens<sizeof...(Subscribers)>::type());
    }

    /* can't figure out how to support
    template<typename... NewSubscribers>
    void subscribe(NewSubscribers... subscribers)
    {
        auto newTuple = std::tuple_cat(_subscriberList, std::tuple<NewSubscribers...>(subscribers...));
        _newTuple = new auto(newTuple);
    }
    */

private:
    template<typename PayloadType, int ...S>
    void dispatchSeq(std::shared_ptr<PayloadType>& payload, seq<S...>)
    {
        dispatch(payload, std::get<S>(_subscriberList) ...);
    }

    template<typename PayloadType, typename SubscriberType, typename... SubscriberTypeList>
    void dispatch(std::shared_ptr<PayloadType>& payload, SubscriberType& subscriber, SubscriberTypeList... subscriberList)
    {
        subscriber.onData(payload);
        dispatch(payload, subscriberList...);
    }

    template<typename PayloadType, typename SubscriberType>
    void dispatch(std::shared_ptr<PayloadType>& payload, SubscriberType& subscriber)
    {
        subscriber.onData(payload);
    }

    template<typename PayloadType, int ...S>
    void postSeq(std::shared_ptr<PayloadType>& payload, seq<S...>)
    {
        post(payload, std::get<S>(_subscriberList) ...);
    }

    template<typename PayloadType, typename SubscriberType, typename... SubscriberTypeList>
    void post(std::shared_ptr<PayloadType>& payload, SubscriberType& subscriber, SubscriberTypeList... subscriberList)
    {
        getIoService().post( [&subscriber, payload](){
            subscriber.onData(payload);
        });
        post(payload, subscriberList...);
    }

    template<typename PayloadType, typename SubscriberType>
    void post(std::shared_ptr<PayloadType>& payload, SubscriberType& subscriber)
    {
        getIoService().post( [&subscriber, payload](){
            subscriber.onData(payload);
        });
    }

    std::tuple<Subscribers...>              _subscriberList;
};

} // namespace vf_common



#endif /* STATICSIGNAL_H_ */
