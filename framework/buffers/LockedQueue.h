/*
 * LockedQueues.h
 *
 *  Created on: 23 Jan 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef LOCKEDQUEUE_H_
#define LOCKEDQUEUE_H_

#include <boost/thread/mutex.hpp>
#include <queue>

namespace vf_common
{

template<typename ElementType>
class LockedQueue
{
public:
    typedef ElementType                 value_type;
    typedef std::shared_ptr<value_type> value_ptr_type;

    LockedQueue()
    {
    }

    ~LockedQueue()
    {
    }

    bool pop(value_ptr_type& element)
    {
        boost::mutex::scoped_lock lock(_mutex);
        if(_queue.empty())
        {
            return false;
        }

        element = _queue.front();
        _queue.pop();
        return true;
    }

    void push(value_ptr_type& element)
    {
        boost::mutex::scoped_lock lock(_mutex);
        _queue.push(element);
    }

    bool is_lock_free (void) const
    {
        return false;
    }

    void clear()
    {
        boost::mutex::scoped_lock lock(_mutex);
        while(!_queue.empty())
        {
            _queue.pop();
        }
    }

    bool empty()
    {
        boost::mutex::scoped_lock lock(_mutex);
        return _queue.empty();
    }

    size_t size()
    {
        boost::mutex::scoped_lock lock(_mutex);
        return _queue.size();
    }

    static value_ptr_type createOne()
    {
        return std::make_shared<value_type>();
    }

private:
    std::queue<value_ptr_type>    _queue;
    boost::mutex                  _mutex;
};

} // namespace vf_common

#endif /* LOCKEDQUEUE_H_ */
