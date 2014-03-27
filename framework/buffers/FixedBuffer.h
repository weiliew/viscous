/*
 * FixedBuffer.h
 *
 *  Created on: 18 Jan 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXEDBUFFER_H_
#define FIXEDBUFFER_H_

#include "utilities/Utilities.h"
#include "BufferFactory.h"

namespace vf_common
{

template<size_t BufferSize>
class FixedBuffer
{
public:
    typedef std::array<char, BufferSize>            storage_type;
    typedef typename storage_type::value_type       value_type;
    typedef typename storage_type::size_type        size_type;

    typedef typename storage_type::iterator         iterator;
    typedef typename storage_type::const_iterator   const_iterator;

    FixedBuffer()
    {
    }

    ~FixedBuffer()
    {
    }

    FixedBuffer(const FixedBuffer<BufferSize>& copy)
    {
        std::copy(copy.buffer_.begin(), copy.buffer_.end(), buffer_.begin());
    }

    /* TODO
    FixedBuffer(FixedBuffer<BufferSize>&& move)
    {
    }
    */

    void push(const value_type& t)
    {
        if(likely(bufferSize_ < BufferSize))
        {
            buffer_.push(t);
            ++bufferSize_;
        }
    }

    void pop()
    {
        if(likely(bufferSize_ > 0))
        {
            --bufferSize_;
            buffer_.pop();
        }
    }
/*
    value_type& top()
    {
        if(unlikely(bufferSize_ == 0))
        {
            return value_type();
        }

        return buffer_[0];
    }

    const value_type& top() const
    {
        if(unlikely(bufferSize_ == 0))
        {
            return value_type();
        }

        return buffer_[0];
    }
*/
    size_t size() const
    {
        return bufferSize_;
    }

    bool empty() const
    {
        return !bufferSize_;
    }

    constexpr size_t capacity()
    {
        return BufferSize;
    }

    char * buffer()
    {
        return buffer_.data();
    }

    void setBuffer(const char * buffer, size_t len)
    {
        if(unlikely(len > BufferSize))
        {
            // TODO - truncate and copy ?
            return;
        }

        memcpy(buffer_.data(), buffer, len); // TODO memcpy or std::copy
        bufferSize_ = len;
    }

    void setBuffer(const char * buffer)
    {
        if(unlikely(!buffer))
        {
            // TODO - truncate and copy ?
            return;
        }

        size_t len = strlen(buffer);
        bufferSize_ = (len > BufferSize ? BufferSize : len);
        memcpy(buffer_.data(), buffer, bufferSize_); // TODO memcpy or std::copy
    }

    const_iterator begin() const
    {
        return buffer_.begin();
    }

    const_iterator end() const
    {
        return buffer_.end();
    }

    void setSize(size_t size)
    {
        bufferSize_ = size;
    }
/*
    std::ostream& operator<< (const char val[])
    {
        std::ostream toRet;
        return toRet << std::string(buffer_, bufferSize_) << val;
    }

    std::ostream& operator<< (std::ostream& val)
    {
        std::ostream toRet;
        return std::ostream(std::string(buffer_, bufferSize_) << val);
    }
*/
private:
    storage_type                    buffer_;
    size_t                          bufferSize_;
};

}  // namespace vf_common



#endif /* FIXEDBUFFER_H_ */
