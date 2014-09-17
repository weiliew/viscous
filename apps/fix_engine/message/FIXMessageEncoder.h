/*
 * FIXMessageEncoder.h
 *
 *  Created on: 9 Aug 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXMESSAGEENCODER_H_
#define FIXMESSAGEENCODER_H_

#include "CachedField.h"
#include "utilities/Utilities.h"

namespace vf_fix
{

// this class allows one to create a FIX message. This class will store a buffer
// used for the storage of the message, and will keep an array of pointers to the fields
// similar to that in the FIXMessageDecoder for fast access.

// storage of the raw buffer will be in 3 character arrays, one for header, content and trailer respectively
// the purpose of this is to allow writing to the socket more efficient using iovec calls

template<size_t HolderCapacity, size_t BufferSize>
class FIXMessageEncoder
{
public:

    template<size_t Capacity>
    struct FieldGroupType
    {
        // TODO - setField method that sets the pointer and vaalue ...
        // TODO - serialise function to generate output str

        typedef std::pair<int, CachedField>              FieldPairType;
        typedef std::array<FieldPairType, Capacity>      FieldArrType;
        typedef typename FieldArrType::iterator          Iterator;

        FieldGroupType()
        : _numFields(0)
        {
        }

        template<typename FieldType, typename Type>
        bool appendField(FieldType& field, Type& val)
        {
            // append fid
            strncpy(&_buffer[_bufferSize], field.FID_STR, field.size());
            _bufferSize+=field.size();

            // TODO - need to check how efficient this is
            try
            {
                _buffer[_bufferSize] = boost::lexical_cast<char[BuffSz]>(fid);
            }
            catch(const boost::bad_lexical_cast&)
            {
                return false;
            }
        }


        template<typename FieldType>
        bool appendField<FieldType, double>(FieldType& field, double& val)
        {

        }

        template<typename FieldType>
        bool appendField<FieldType, long>(FieldType& field, long& val)
        {

        }

        template<typename FieldType>
        bool appendField<FieldType, bool>(FieldType& field, bool& val)
        {

        }

        template<typename FieldType>
        bool appendField<FieldType, std::string>(FieldType& field, std::string& val)
        {
            // check length
            int fieldLen = field.FID_STR.size() + val.length() + 1;
            if(fieldLen > BuffSz-_bufferSize)
            {
                return false;
            }

            snprintf();
        }

        FieldArrType    _fieldArray;
        size_t          _numFields;
    };



    FIXMessageEncoder()
    {
        // we pre-create the necessary FIX message header
    }

    ~FIXMessageEncoder()
    {
    }

    // note - it is assumed that the order of the add being called will determine the order of the
    // buffer being written


private:
    FieldGroupType<128, 1024>                   _headerFields;
    FieldGroupType<24,   256>                   _trailerFields;
    FieldGroupType<HolderCapacity, BufferSize>  _payloadFields;
};
} // vf_fix


#endif /* FIXMESSAGEENCODER_H_ */
