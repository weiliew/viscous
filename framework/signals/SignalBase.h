/*
 * SignalBase.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef SIGNALBASE_H_
#define SIGNALBASE_H_

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <stdio.h>
#include <vector>
#include "SignalFactory.h"

namespace vf_common
{

class SignalBase
{
public:
    SignalBase(const char * name, boost::asio::io_service& ioService)
    : _name(name)
    , _io(ioService)
    , _strandCount(0)
    {
    }

    virtual ~SignalBase()
    {
        _io.stop();
        _threadPool.join_all();
    }

    void run(size_t numThreads = 1)
    {
        for(size_t count=0;count<numThreads;++count)
        {
            _threadPool.create_thread(boost::bind(&boost::asio::io_service::run, &_io));
        }
    }

    int numRunningThreads()
    {
        return _threadPool.size();
    }

    const char * getName()
    {
        return _name.c_str();
    }

    void setName(const char * name)
    {
        _name = name;
    }

    boost::asio::io_service& getIoService()
    {
        return _io;
    }

    std::shared_ptr<boost::asio::io_service::strand> getStrand(size_t strandId)
    {
        auto findIter = _strandIdMap.find(strandId);
        if(findIter == _strandIdMap.end())
        {
            _strandIdMap.insert(std::make_pair(strandId, _strandCount));
            std::shared_ptr<boost::asio::strand> newStrand = std::make_shared<boost::asio::strand>(_io);
            _strandVec.push_back(newStrand);
            ++_strandCount;
            return newStrand;
        }

        return _strandVec[findIter->second];
    }

private:
    std::string                                         _name;
    boost::asio::io_service&                            _io;
    boost::thread_group                                 _threadPool;
    std::vector<std::shared_ptr<boost::asio::strand> >  _strandVec;
    size_t                                              _strandCount;
    std::map<size_t, size_t>                            _strandIdMap;
};

}  // namespace vf_common


#endif /* SIGNALBASE_H_ */
