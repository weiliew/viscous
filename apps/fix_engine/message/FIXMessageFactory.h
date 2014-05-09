/*
 * FIXMessageFactory.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXMESSAGEFACTORY_H_
#define FIXMESSAGEFACTORY_H_

#include "buffers/BufferFactory.h"

namespace vf_fix
{

using vf_common::PooledBufferFactory;

template<typename PoolType>
class FIXMessageFactory : public PooledBufferFactory<PoolType>
{
public:
    typedef typename PooledBufferFactory<PoolType>::value_ptr_type value_ptr_type;

    FIXMessageFactory(size_t initialSize = 0)
    : PooledBufferFactory<PoolType>(0)
    , _cacheMessage(PooledBufferFactory<PoolType>::create())
    {
    }

    ~FIXMessageFactory()
    {
    }

    value_ptr_type create()
    {
        value_ptr_type toRet;
        if(_cacheMessage->size() > 0)
        {
            toRet = PooledBufferFactory<PoolType>::clone(_cacheMessage);
            _cacheMessage->clear();
        }
        else
        {
            toRet = PooledBufferFactory<PoolType>::create();
        }

        // TODO - how to set factry into the fix message ?
        // toRet->setFactory(this);

        return toRet;
    }

    void destroy(value_ptr_type buffer)
    {
        buffer->clear();
        PooledBufferFactory<PoolType>::destroy(buffer);
    }

    void cachePartialMessage(value_ptr_type partialMsg, size_t offset)
    {
        if(LIKELY(partialMsg->size() > offset))
        {
            _cacheMessage.appendBuffer(partialMsg->buffer()+offset, partialMsg->size() - offset);
        }
    }

    size_t getCachedSize()
    {
        return _cacheMessage->size();
    }

private:
    // cache message to store partial message from previous invocation
    value_ptr_type _cacheMessage;
};

}  // namespace vf_fix



#endif /* FIXMESSAGEFACTORY_H_ */
