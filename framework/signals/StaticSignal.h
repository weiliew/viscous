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

#include <type_traits>
#include <tuple>
#include <iostream>
#include "SignalBase.h"

// TODO - Replace Signal and StaticSignal with co-routine implementation for flexibility and speed

namespace vf_common
{

template<typename PayloadType, typename... Subscribers>
class StaticSignal : public SignalBase
{
public:
    struct Traits
    {
        typedef std::false_type DynamicSignal;
    };

    typedef PayloadType                 element_type;
    typedef std::tuple<Subscribers...>  TupleType;

    StaticSignal(const char * name, boost::asio::io_service& ioService, Subscribers&... subscriberList)
    : SignalBase(name, ioService)
    , _subscriberList(std::make_tuple(std::ref(subscriberList)...))
    {
    }

    virtual ~StaticSignal()
    {
    }

    inline void post(PayloadType& payload)
    {
        postSeq(payload, typename gens<sizeof...(Subscribers)>::type());
    }

    void dispatch(PayloadType& payload)
    {
        dispatchSeq(payload, typename gens<sizeof...(Subscribers)>::type());
    }

    template<typename... NewSubscribers>
    void subscribe(NewSubscribers&... subscribers) = delete;
    /* not possible ?! note - may want to modify to make use of lamda-over-lamda/monad type implementation
    {
        auto newTuple = std::tuple_cat(_subscriberList, std::tuple<NewSubscribers...>(subscribers...));
    }
    */

    template<typename SubscriberCheckType>
    bool isPartOf()
    {
        // note - this only checks for matching type only
        return isPartOfSeq<SubscriberCheckType>(typename gens<sizeof...(Subscribers)>::type());
    }

private:
    template<typename SubscriberCheckType, int ...S>
    bool isPartOfSeq(seq<S...>)
    {
        return isPartOf<SubscriberCheckType>(std::get<S>(_subscriberList) ...);
    }

    template<typename SubscriberCheckType, typename SubscriberType, typename... SubscriberTypeList>
    bool isPartOf(SubscriberType&, SubscriberTypeList... subscriberList)
    {
        if(std::is_same<SubscriberCheckType, SubscriberType>::value)
        {
            return true;
        }

        return isPartOf<SubscriberCheckType>(subscriberList...);
    }

    template<typename SubscriberCheckType, typename SubscriberType>
    bool isPartOf(SubscriberType& subscriber)
    {
        return std::is_same<SubscriberCheckType, SubscriberType>::value;
    }

    template<int ...S>
    void dispatchSeq(PayloadType& payload, seq<S...>)
    {
        dispatch(payload, std::get<S>(_subscriberList) ...);
    }

    template<typename SubscriberType, typename... SubscriberTypeList>
    void dispatch(PayloadType& payload, SubscriberType& subscriber, SubscriberTypeList... subscriberList)
    {
        subscriber.onData(payload);
        dispatch(payload, subscriberList...);
    }

    template<typename SubscriberType>
    void dispatch(PayloadType& payload, SubscriberType& subscriber)
    {
        subscriber.onData(payload);
    }

    template<int ...S>
    void postSeq(PayloadType& payload, seq<S...>)
    {
        post(payload, std::get<S>(_subscriberList) ...);
    }

    template<typename SubscriberType, typename... SubscriberTypeList>
    void post(PayloadType& payload, SubscriberType& subscriber, SubscriberTypeList... subscriberList)
    {
        getIoService().post( [&subscriber, &payload](){
            subscriber.onData(payload);
        });
        post(payload, subscriberList...);
    }

    template<typename SubscriberType>
    void post(PayloadType& payload, SubscriberType& subscriber)
    {
        getIoService().post( [&subscriber, &payload](){
            subscriber.onData(payload);
        });
    }

    std::tuple<Subscribers...>              _subscriberList;
};

} // namespace vf_common



#endif /* STATICSIGNAL_H_ */
