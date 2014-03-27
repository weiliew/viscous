/*
 * LockFreeQueue.h
 *
 *  Created on: 23 Mar 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef LOCKFREEQUEUE_H_
#define LOCKFREEQUEUE_H_


#include <boost/thread/mutex.hpp>
#include <queue>
#include <atomic>

namespace vf_common
{

template<typename ElementType, size_t InitialCapacity = 1024>
class LockFreeQueue
{
public:
    typedef ElementType                 value_type;
    typedef ElementType*                value_ptr_type;

    LockFreeQueue()
    : _queue(InitialCapacity)
    , _queueSize(0)
    {
    }

    ~LockFreeQueue()
    {
    }

    bool pop(value_ptr_type& element)
    {
        if(unlikely(!_queueSize))
        {
            return false;
        }

        --_queueSize;
        return _queue.pop(element);
    }

    void push(value_ptr_type& element)
    {
        ++_queueSize;
        _queue.push(element);
    }

    bool is_lock_free (void) const
    {
        return _queue.is_lock_free();
    }

    void clear()
    {
        value_ptr_type dummy;
        while(_queue.pop(dummy))
        {
            delete dummy;
        }
        _queueSize = 0;
    }

    bool empty()
    {
        return _queue.empty();
    }

    size_t size()
    {
        return _queueSize;
    }

    static value_ptr_type createOne()
    {
        // TODO - might want to specify an allocator
        return new value_type();
    }

private:
    boost::lockfree::stack<value_ptr_type>  _queue;
    std::atomic<int>                        _queueSize;
};

} // namespace vf_common




#endif /* LOCKFREEQUEUE_H_ */
