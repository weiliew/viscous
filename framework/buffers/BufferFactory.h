/*
 * BufferFactory.h
 *
 *  Created on: 19 Feb 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef BUFFERFACTORY_H_
#define BUFFERFACTORY_H_

#include "utilities/Utilities.h"

namespace vf_common
{

template<typename BufferType>
class BufferFactory
{
public:
    typedef BufferType  ElementType;

    BufferFactory()
    {
    }

    ~BufferFactory()
    {
    }

    BufferType * create()
    {
        return new BufferType();
    }

    void destroy(BufferType * buffer)
    {
        delete buffer;
    }

private:

};

template<typename PoolType>
class PooledBufferFactory
{
public:
    typedef typename PoolType::value_type        value_type;
    typedef typename PoolType::value_ptr_type    value_ptr_type;

    PooledBufferFactory(size_t initialSize = 0)
    {
        if(initialSize)
        {
            for(size_t count=0;count<initialSize;++count)
            {
                auto toRelease = PoolType::createOne();
                _pool.release(toRelease);
            }
        }
    }

    ~PooledBufferFactory()
    {
        // clean up
        _pool.clear();
    }

    value_ptr_type create()
    {
        // get from pool or if pool empty, create new
        value_ptr_type toRet;
        if(LIKELY(_pool.aquire(toRet)))
        {
            return toRet;
        }

        return PoolType::createOne();
    }

    value_ptr_type clone(value_ptr_type copy)
    {
        // get from pool or if pool empty, create new
        value_ptr_type toRet;
        if(UNLIKELY(!_pool.aquire(toRet)))
        {
            toRet = PoolType::createOne();
        }
        // make copy
        toRet->copy(*copy);

        return toRet;
    }

    void destroy(value_ptr_type buffer)
    {
        buffer->clear();
        _pool.release(buffer);
    }

    size_t size()
    {
        return _pool.size();
    }

private:
    PoolType    _pool;
};


}  // namespace vf_common



#endif /* BUFFERFACTORY_H_ */
