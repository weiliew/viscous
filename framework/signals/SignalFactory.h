/*
 * SignalFactory.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef SIGNALFACTORY_H_
#define SIGNALFACTORY_H_

#include "utilities/Utilities.h"

namespace vf_common
{

template<typename SignalType, typename... Args>
class SignalFactory
{
public:
    typedef SignalType ProductType;

    SignalFactory(const char * name, boost::asio::io_service& ioService, Args... argsList)
    : _baseIo(ioService)
    , _argsList(std::make_tuple(argsList...))
    {
    }

    SignalFactory(const SignalFactory<SignalType, Args...>& copy)
    : _baseIo(copy._baseIo)
    , _baseName(copy._baseName)
    , _signalList(copy._signalList)
    , _argsList(copy._argsList)
    {

    }

    virtual ~SignalFactory()
    {
        _signalList.clear();
    }

    virtual std::shared_ptr<SignalType> create(boost::asio::io_service* io = NULL)
    {
        boost::asio::io_service * useIo = (io == NULL ? &_baseIo : io);
        char sigName[64];
        snprintf((char *)&sigName, 64, "%s_%u", _baseName.c_str(), (unsigned int) _signalList.size()+1);
        std::shared_ptr<SignalType> newSig(createNewSignal(sigName, *useIo, typename gens<sizeof...(Args)>::type()));
        _signalList.push_back(newSig);
        return newSig;
    }

    void setArgsList(Args... argsList)
    {
        _argsList = std::make_tuple(argsList...);
    }

private:
    template<int ...S>
    SignalType * createNewSignal(const char * sigName, boost::asio::io_service& io, seq<S...>)
    {
        return createNewSignal(sigName, io, std::get<S>(_argsList) ...);
    }

    template<typename... SubscriberTypeList>
    SignalType * createNewSignal(const char * sigName, boost::asio::io_service& io, SubscriberTypeList... argsList)
    {
        return new SignalType(sigName, io, argsList...);
    }

    boost::asio::io_service&                    _baseIo;
    std::string                                 _baseName;
    std::vector<std::shared_ptr<SignalType> > _signalList;
    std::tuple<Args...>                         _argsList;
};

}  // namespace vf_common


#endif /* SIGNALFACTORY_H_ */
