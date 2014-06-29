/*
 * FIXMessageDecoder.h
 *
 *  Created on: 27 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXMESSAGEDECODER_H_
#define FIXMESSAGEDECODER_H_

#define SOH 0x01
#define EQ  '='

#include "utilities/Utilities.h"

namespace vf_fix
{

// this class allows a single FIX message to be decoded and the FIX fields to be walked by the caller
// Its intention is that the caller will have a re-usable decoder to parse all the incoming messages
template<size_t HolderCapacity>
class FIXMessageDecoder
{
public:
    typedef std::pair<const char *, const char *>       FieldPairType;
    typedef std::array<FieldPairType, HolderCapacity>   FieldArrType;
    typedef typename FieldArrType::iterator             Iterator;

    FIXMessageDecoder()
    : _numFields(0)
    , _currPos(0)
    , _parsed(false)
    {
        _fieldArrays.fill(std::make_pair((const char *) NULL, (const char *) NULL));
    }

    virtual ~FIXMessageDecoder(){}

    bool parseBuffer(char * buffer, size_t len)
    {
        if(UNLIKELY(buffer == NULL))
        {
            return false;
        }

        char * idx = buffer;
        size_t consumed = 0;
        while(consumed < len)
        {
            _fieldArrays[_numFields].first = idx;
            idx = (char *) memchr(idx, EQ, len - consumed);
            if(UNLIKELY(idx == NULL))
            {
                return false;
            }
            *idx = '\0';
            ++idx;
            _fieldArrays[_numFields].second = idx;
            consumed = idx-buffer;
            idx = (char *) memchr(idx, SOH, len - consumed);
            if(UNLIKELY(idx == NULL))
            {
                return false;
            }
            *idx = '\0';
            ++idx;
            consumed = idx-buffer;
            ++_numFields;
        }

        _parsed = true;
        return true;
    }

    bool next(bool reset = false)
    {
        if(reset)
        {
            _currPos = 0;
            return true;
        }

        if(_currPos+1 >= _numFields)
        {
            return false;
        }

        ++_currPos;
        return true;
    }

    void rewind()
    {
        if(_currPos > 0)
        {
            --_currPos;
        }
    }

    void reset()
    {
        _currPos = 0;
    }

    FieldPairType& currentField()
    {
        return _fieldArrays[_currPos];
    }

    std::string toString()
    {
        std::ostringstream oss;
        for(unsigned int count=0;count<_numFields;++count)
        {
            oss << _fieldArrays[count].first;
            oss << "=";
            oss << _fieldArrays[count].second;
            oss << "|";
        }

        return oss.str();
    }

private:
    FieldArrType   _fieldArrays;
    size_t         _numFields;
    size_t         _currPos;
    bool           _parsed;
};

}  // namespace vf_fix



#endif /* FIXMESSAGEDECODER_H_ */
