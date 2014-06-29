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
#define EQ  '='

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

    FIXMsg()
    : _complete   (false)
    {
        memset(&_scratchSpace, 0, sizeof(_scratchSpace));
    }


    FIXMsg(const FIXMsg<BufferType>& copy)
    : _bufferStore  (copy._bufferStore)
    , _complete     (copy._complete)
    {
        memset(&_scratchSpace, 0, sizeof(_scratchSpace));
    }

    FIXMsg<BufferType>& operator=(const FIXMsg<BufferType>& copy)
    {
        if(this != &copy)
        {
            _bufferStore    = copy._bufferStore;
            _complete       = copy._complete;
            memcpy(&_scratchSpace, &copy._scratchSpace, sizeof(_scratchSpace));
        }
        return *this;
    }

    void copy(const FIXMsg<BufferType>& copy)
    {
        if(this != &copy)
        {
            _bufferStore.copy(copy._bufferStore);
            _complete = copy._complete;
            memcpy(&_scratchSpace, &copy._scratchSpace, sizeof(_scratchSpace));
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
    template<typename DecoderType>
    bool parseFIXMessage(DecoderType& decoder)
    {
        if(UNLIKELY(!_complete))
        {
            return false;
        }

        return decoder.parseBuffer(buffer(), size());
    }

    /* Attempts to identify a complete fix message in the buffer
     * Returns
     *      the number of bytes consumes from the buffer if a complete fix message is available in the buffer
     *      0 if buffer does not contain a complete fix
     *      -1 if buffer contains an invalid fix message
     */
    int getCompleteMsg()
    {
        char * buffer = _bufferStore.buffer();
        size_t len = _bufferStore.size();

        // parse and set the fix message - first check if we got the full message
        // always start with 8=FIX...^9=Len

        // sanity check to see if we got enough data
        if(len < 10)
        {
            return 0; // don't have a complete message
        }

        // find start of message
        const unsigned char * sep = (const unsigned char *) memchr(buffer, SOH, len);
        if(!sep)
        {
            return 0; // don't have a complete message
        }
        ++sep;

        // get the msg len
        const unsigned char * sepEnd = (const unsigned char *) memchr(sep, SOH, len-((char*)sep-buffer));
        if(!sepEnd)
        {
            return 0; // don't have a complete message
        }

        // verify FIX message
        if(sep[0] != '9' || buffer[0] != '8')
        {
            return -1; // not a valid fix message
        }
        sep+=2; // move to the size value

        memcpy(&_scratchSpace, sep, sepEnd-sep);
        int totalLenNeeded = atoi(_scratchSpace) + ((char*)sepEnd-buffer) + 7; // plus checksum 10=xxx|

        if(totalLenNeeded > len)
        {
            return 0; // still don't have a complete message
        }

        // got a complete FIX message from this point onwards

        // message does not end with SOH, invalid data ?! we are adding our own SOH for now.
        if(buffer[totalLenNeeded-1] != SOH)
        {
            // we need SOH at the end of the FIX message for faster parsing
            char * endChar = _bufferStore.buffer()+totalLenNeeded-1;
            *endChar = SOH;
        }

        _complete = true;
        return totalLenNeeded;
    }

    // interface for getting the underlying buffer store - for direct set and get
    BufferType& getBufferStore()
    {
        return _bufferStore;
    }

    // Forwarding function calls to buffer storage

    void clear()
    {
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

    void appendBuffer(const char * buffer, size_t len)
    {
        _bufferStore.appendBuffer(buffer, len);
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

private:
    char                _scratchSpace[24];
    BufferType          _bufferStore;
    bool                _complete;
};

}  // namespace osf_fix


#endif /* FIXMESSAGE_H_ */
