/*
 * BufferPool.h
 *
 *  Created on: 18 Jan 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef BUFFERPOOL_H_
#define BUFFERPOOL_H_

namespace vf_common
{

template<typename QueueType>
class BufferPool
{
public:
    typedef typename QueueType::value_type      value_type;
    typedef typename QueueType::value_ptr_type  value_ptr_type;

    BufferPool()
    {
    }

    ~BufferPool()
    {
    }

    bool aquire(value_ptr_type& value)
    {
        return _bufferPool.pop(value);
    }

    void release(value_ptr_type& value)
    {
        _bufferPool.push(value);
    }

    void clear()
    {
        _bufferPool.clear();
    }

    size_t size()
    {
        return _bufferPool.size();
    }

    bool empty()
    {
        return _bufferPool.empty();
    }

    static value_ptr_type createOne()
    {
        return QueueType::createOne();
    }

private:
    QueueType   _bufferPool;
};

}  // namespace vf_common


#endif /* BUFFERPOOL_H_ */
