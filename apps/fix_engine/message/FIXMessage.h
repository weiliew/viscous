/*
 * FIXMessage.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef FIXMESSAGE_H_
#define FIXMESSAGE_H_

#include "FIXMessageFactory.h"

using namespace vf_common;

#define SOH 0x01

namespace vf_fix
{

// FIXMsg is a holder class for fix message containing the parsed message buffer
// with references to the field mappings for quick lookup and setters
// we are not inheriting from FixedBuffer as we need FIXMsg to be a POD type

template<typename BufferType>
class FIXMsg
{
public:
    typedef BufferType                              storage_type;
    typedef typename storage_type::value_type       value_type;
    typedef typename storage_type::size_type        size_type;

    typedef typename storage_type::iterator         iterator;
    typedef typename storage_type::const_iterator   const_iterator;

    // TODO - add variadic template paraams to the factory create function

    FIXMsg()
    //: _factory    (NULL)
    : _complete   (false)
    , _parsed     (false)
    {
    }


    FIXMsg(const FIXMsg<BufferType>& copy)
    //: _factory      (copy._factory)
    : _bufferStore  (copy._bufferStore)
    , _complete     (copy._complete)
    , _parsed       (copy._parsed)
    {
    }

    FIXMsg<BufferType>& operator=(const FIXMsg<BufferType>& copy)
    {
        if(this != &copy)
        {
            //_factory        = copy._factory;
            _bufferStore    = copy._bufferStore;
            _complete       = copy._complete;
            _parsed         = copy._parsed;
        }
        return *this;
    }

    void copy(const FIXMsg<BufferType>& copy)
    {
        if(this != &copy)
        {
            //_factory  = copy._factory;
            _bufferStore.copy(copy._bufferStore);
            _complete = copy._complete;
            _parsed   = copy._parsed;
        }
    }

    /* TODO
    FIXMsg<BufferType>(FIXMsg<BufferType>&& move)
    {
    }
    */

    ~FIXMsg()
    {
        clear();
    }

    // parse the FIX message - returns false if fix message is not valid, or if a complete fix message is not available
    bool parseFIXMessage()
    {
        if(UNLIKELY(!_complete))
        {
            return false;
        }

        // Replace all SOH with NULLs, and locate each field offset
        // TODO -

        _parsed = true;
        return true;
    }

    // returns true if a complete FIX message is available in the buffer, stores the rest of the buffer back into the
    // cache store in the factory for next use
    bool getFIXMsg()
    {
        char * buffer = _bufferStore.buffer();
        size_t len = _bufferStore.size();

        // parse and set the fix message - first check if we got the full message
        // always start with 8=FIX...^9=Len

        // sanity check to see if we got enough data
        if(len < 10)
        {
            /* TODO
            if(LIKELY(_factory))
            {
                _factory->cachePartialMessage(_bufferStore, 0);
            }
            */
            return false; // don't have a complete message
        }

        // find start of message
        const unsigned char * sep = (const unsigned char *) memchr(buffer, SOH, len);
        if(!sep)
        {
            /* TODO
            if(LIKELY(_factory))
            {
                _factory->cachePartialMessage(_bufferStore, 0);
            }
            */
            return false; // don't have a complete message
        }
        ++sep;

        // get the msg len
        const unsigned char * sepEnd = (const unsigned char *) memchr(sep, SOH, len-((char*)sep-buffer));
        if(!sepEnd)
        {
            /* TODO
            if(LIKELY(_factory))
            {
                _factory->cachePartialMessage(_bufferStore, 0);
            }
            */
            return false; // don't have a complete message
        }

        // verify FIX message
        if(sep[0] != '9' || buffer[0] != '8')
        {
            // invalid FIX message
            return false;
        }
        sep+=2; // move to the size value

        char strLen[24];
        memcpy(&strLen, sep, sepEnd-sep);
        int totalLenNeeded = atoi(strLen) + ((char*)sepEnd-buffer) + 7; // plus checksum 10=xxx|

        if(totalLenNeeded > len)
        {
            /* TODO
            if(LIKELY(_factory))
            {
                _factory->cachePartialMessage(_bufferStore, 0);
            }
            */
            return false; // don't have a complete message
        }

        // got a complete FIX message, store the remainder
        if(len > totalLenNeeded)
        {
            /* TODO
            if(LIKELY(_factory))
            {
                _factory->cachePartialMessage(_bufferStore, totalLenNeeded);
            }
            */
        }

        // message does not end with SOH, invalid data ?! we are adding our own SOH for now.
        if(buffer[totalLenNeeded-1] != SOH)
        {
            // we need SOH at the end of the FIX message for faster parsing
            char * endChar = _bufferStore.buffer()+totalLenNeeded-1;
            *endChar = SOH;
        }

        _complete = true;
        return true;
    }

    // interface for getting the underlying buffer store - for direct set and get
    BufferType& getBufferStore()
    {
        return _bufferStore;
    }

    // Forwarding function calls to buffer storage

    void clear()
    {
        _parsed = false;
        _complete = false;
        _bufferStore.clear();
    }

    void push(const value_type& t)
    {
        _bufferStore.push(t);
    }

    void pop()
    {
        _bufferStore.pop();
    }

    size_t size() const
    {
        return _bufferStore.size();
    }

    bool empty() const
    {
        return _bufferStore.empty();
    }

    constexpr size_t capacity()
    {
        return _bufferStore.capacity();
    }

    char * buffer()
    {
        return _bufferStore.buffer();
    }

    void setBuffer(const char * buffer, size_t len)
    {
        _bufferStore.setBuffer(buffer, len);
    }

    void setBuffer(const char * buffer)
    {
        _bufferStore.setBuffer(buffer);
    }

    const_iterator begin() const
    {
        return _bufferStore.begin();
    }

    const_iterator end() const
    {
        return _bufferStore.end();
    }

    void setSize(size_t size)
    {
        _bufferStore.setSize(size);
    }

    bool complete()
    {
        return _complete;
    }

    bool parsed()
    {
        return _parsed;
    }

    /* TODO
    void setFactory(FIXMessageFactory* factory)
    {
        _factory = factory;
    }
    */

private:
    // TODO - FIXMessageFactory*  _factory;
    BufferType          _bufferStore;
    bool                _complete;
    bool                _parsed;
};

}  // namespace osf_fix


#endif /* FIXMESSAGE_H_ */
