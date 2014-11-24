/*
 * SFIXMessages.h
 *
 *  Created on: 29 May 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef SFIXMESSAGES_H_
#define SFIXMESSAGES_H_

#include <sys/uio.h>
#include "boost/asio.hpp"

namespace vf_fix
{

// TODO - deal with this ?
constexpr int HeaderReservedCount = 20;

template<const StringConstant&  Name,
         const StringConstant&  Type,
         typename               Validate,
         typename               HeaderType,
         typename               TrailerType,
         typename...            FieldTypes>
class SFIXMessage
{
public:
    constexpr static StringConstant        NAME        = Name;
    constexpr static StringConstant        TYPE        = Type;
    constexpr static bool                  VALIDATE    = Validate::value;
    constexpr static size_t                VEC_CAPACITY = sizeof...(FieldTypes)*3 + HeaderType::VEC_CAPACITY + TrailerType::VEC_CAPACITY;
    constexpr static size_t                OUTPUT_STR_CAPACITY = VEC_CAPACITY*24; // TODO - might want to make this a template var

    SFIXMessage()
    {
    }

    ~SFIXMessage()
    {
    }

    template<typename DecoderType>
    bool set(DecoderType& decoder)
    {
        if(UNLIKELY(!decoder.currentField().first || !decoder.currentField().second))
        {
            return false;
        }

        // set headers
        if(!_header.set(decoder))
        {
            return false;
        }

        decoder.next();
        bool msgComplete = false;
        do
        {
            if(!setSubField(decoder))
            {
                // are we at the end of message ?
                if(_trailer.set(decoder))
                {
                    // make sure we are at the end of the message that was passed in
                    if(decoder.isLast())
                    {
                        // should not be any fields left ?!
                        return false;
                    }
                    msgComplete = true;
                    break; // we're done
                }

                // else error
                return false;
            }
        } while(decoder.next());

        // optional validation check
        return validate<VALIDATE>(msgComplete);
    }

    template<typename DecoderType>
    bool setSubField(DecoderType& decoder)
    {
        return setSubFieldUnwind(decoder, typename gens<sizeof...(FieldTypes)>::type());
    }

    constexpr bool isSubField(int fid)
    {
        return isSubFieldUnwind(fid, typename gens<sizeof...(FieldTypes)>::type());
    }

    template<typename T, typename ToString = std::true_type>
    bool setSubField(int fid, T& val)
    {
        return setSubFieldUnwind<ToString>(fid, val, typename gens<sizeof...(FieldTypes)>::type());
    }

    constexpr bool isHeaderField(int fid)
    {
        return _header.isSubField(fid);
    }

    constexpr bool isTrailerField(int fid)
    {
        return _trailer.isSubField(fid);
    }

    std::ostringstream& toString(std::ostringstream& os)
    {
        _header.toString(os);
        toStringUnwind(os, typename gens<sizeof...(FieldTypes)>::type());
        return _trailer.toString(os);
    }

    bool getSubField(int fid, CachedField& retField)
    {
        return getSubFieldUnwind(fid, retField, typename gens<sizeof...(FieldTypes)>::type());
    }

    HeaderType& header()
    {
        return _header;
    }

    TrailerType& trailer()
    {
        return _trailer;
    }

    template<typename GroupType>
    bool getSubGroup(int fid, GroupType& retField)
    {
        return getSubGroupUnwind(fid, retField, typename gens<sizeof...(FieldTypes)>::type());
    }

    void clear()
    {
        _header.clear();
        _trailer.clear();
        clearUnwind();
    }

    // generates and returns a vector of char arrays for fields contained in this message
    /* TODO - not supported yet
    const iovec* generateFIXStringVec(size_t& countVec, bool force = false)
    {
        // check if this has been generated before
        if(!_vecGenerated || force)
        {
            iovec* vec = &_ioVec[0];

            _vecGenerated =
                _header.setIoVec(vec) &&
                setIoVec(vec)         &&
                _trailer.setIoVec(vec);

            if(_vecGenerated)
            {
                // set end of vec
                vec->iov_base = NULL;
                vec->iov_len = 0;
                _countVec = (vec - &_ioVec[0])/sizeof(iovec);
                ++vec;
                return _ioVec;
            }
            else
            {
                return NULL;
            }
        }

        countVec = _countVec;
        return _ioVec;
    }
    */

    boost::asio::const_buffer getBufferOutput(bool force = false)
    {
        int startIdx = HeaderReservedCount;

        if(!_strGenerated)
        {
            int remLen = OUTPUT_STR_CAPACITY - startIdx;
            char * buffer = &_outputStr[startIdx];

            // add msg type - msg type in the format of <SOH>35=<MsgType><SOH>
            memcpy(buffer, TYPE.data(), TYPE.size());
            remLen -= TYPE.size();
            buffer += TYPE.size();

            _strGenerated = _header.setOutputBuffer(buffer, remLen) &&
                             setOutputBuffer(buffer, remLen) &&
                            _trailer.setOutputBuffer(buffer, remLen);
            if(_strGenerated)
            {
                _outputStrLen = OUTPUT_STR_CAPACITY - remLen;
            }
            else
            {
                // nothing in the message - return an empty buffer
                return boost::asio::const_buffer();
            }
        }

        // now append the begin string and the length
        if(_outputStrLen < 10)
        {
            startIdx -= 2;
        }
        else if(_outputStrLen < 100)
        {
            startIdx -= 3;
        }
        else if(_outputStrLen < 1000)
        {
            startIdx -= 4;
        }
        else
        {
            assert(false); // hmmm really ?
            return boost::asio::const_buffer();
        }

        // apply length - TODO - any faster way of doing this ?
        sprintf(&_outputStr[startIdx], "%d", _outputStrLen);
        int bufferStartIdx = startIdx - fix_defs::BeginString.size();
        memcpy(&_outputStr[bufferStartIdx], fix_defs::BeginString.data(), fix_defs::BeginString.size());

        // TODO - checksum !!

        return boost::asio::const_buffer(&_outputStr[bufferStartIdx], _outputStrLen + (HeaderReservedCount - bufferStartIdx));
    }

private:
    template<bool T>
    typename std::enable_if<T, bool>::type validate(bool msgComplete)
    {
        if(!msgComplete)
        {
            return false;
        }

        return checkRequired();
    }

    template<bool T>
    typename std::enable_if<!T, bool>::type validate(bool msgComplete)
    {
        return msgComplete;
    }

    // setOutputBuffer
    bool setOutputBuffer(char *& buffer, int& remLen)
    {
        return setOutputBufferUnwind(buffer, remLen, typename gens<sizeof...(FieldTypes)>::type());
    }

    template<int ...S>
    bool setOutputBufferUnwind(char *& buffer, int& remLen, seq<S...>)
    {
        return setOutputBuffer(buffer, remLen, std::get<S>(_fieldList) ...);
    }

    template<typename FieldType, typename... FieldTypeList>
    bool setOutputBuffer(char *& buffer, int& remLen, FieldType& field, FieldTypeList&... fieldList)
    {
        if(field.isSet())
        {
            if(!field.setOutputBuffer(buffer, remLen))
            {
                return false;
            }
        }
        return setOutputBuffer(buffer, remLen, fieldList...);
    }

    template<typename FieldType>
    bool setOutputBuffer(char *& buffer, int& remLen, FieldType& field)
    {
        if(field.isSet())
        {
            if(!field.setOutputBuffer(buffer, remLen))
            {
                return false;
            }
        }

        return true;
    }

    // setIoVec
    // sets the iovec structure passed in
    bool setIoVec(iovec*& vec)
    {
        return setIoVecUnwind(vec, typename gens<sizeof...(FieldTypes)>::type());
    }

    template<int ...S>
    bool setIoVecUnwind(iovec*& vec, seq<S...>)
    {
        return setIoVec(vec, std::get<S>(_fieldList) ...);
    }

    template<typename FieldType, typename... FieldTypeList>
    bool setIoVec(iovec*& vec, FieldType& field, FieldTypeList&... fieldList)
    {
        if(!field.setIoVec(vec))
        {
            return false;
        }

        return setIoVec(vec, fieldList...);
    }

    template<typename FieldType>
    bool setIoVec(iovec*& vec, FieldType& field)
    {
        return field.setIoVec(vec);
    }

    // setSubField
    template<typename DecoderType, int ...S>
    bool setSubFieldUnwind(DecoderType& decoder, seq<S...>)
    {
        return setSubField(decoder, decoder.currentField().first, std::get<S>(_fieldList) ...);
    }

    template<typename DecoderType, typename FieldType, typename... FieldTypeList>
    bool setSubField(DecoderType& decoder, int fid, FieldType& field, FieldTypeList&... fieldList)
    {
        if(fid == field.FID)
        {
            // found it
            return field.set(decoder);
        }

        return setSubField(decoder, fid, fieldList...);
    }

    template<typename DecoderType, typename FieldType>
    bool setSubField(DecoderType& decoder, int fid, FieldType& field)
    {
        if(fid == field.FID)
        {
            // found it
            return field.set(decoder);
        }
        else
        {
            // not found
            return false;
        }
    }

    template<typename ToString, typename T, int ...S>
    bool setSubFieldUnwind(int fid, T& val, seq<S...>)
    {
        return setSubField<ToString>(fid, val, std::get<S>(_fieldList) ...);
    }

    template<typename ToString, typename T, typename FieldType, typename... FieldTypeList>
    bool setSubField(int fid, T& val, FieldType& field, FieldTypeList&... fieldList)
    {
        if(fid == field.FID)
        {
            // found it
            setField<T, FieldType, ToString>(val, field);
            return true;
        }

        return setSubField<ToString>(fid, val, fieldList...);
    }

    template<typename ToString, typename T, typename FieldType>
    bool setSubField(int fid, T& val, FieldType& field)
    {
        if(fid == field.FID)
        {
            // found it
            setField<T, FieldType, ToString>(val, field);
            return true;
        }
        else
        {
            // not found
            return false;
        }
    }

    template<typename T, typename FieldType, typename ToString>
    typename std::enable_if<std::is_array<T>::value, void>::type
    setField(T& val, FieldType& field)
    {
        field.setValue(val);
    }

    template<typename T, typename FieldType, typename ToString>
    typename std::enable_if<!std::is_array<T>::value, void>::type
    setField(T& val, FieldType& field)
    {
        field.template setValue<T, ToString>(val);
    }

    // isSubField
    template<int ...S>
    constexpr bool isSubFieldUnwind(int fid, seq<S...>)
    {
        return isSubField(fid, std::get<S>(_fieldList) ...);
    }

    template<typename FieldType, typename... FieldTypeList>
    constexpr bool isSubField(int fid, FieldType& field, FieldTypeList&... fieldList)
    {
        if(fid == field.FID)
        {
            return true;
        }

        return isSubField(fid, fieldList...);
    }

    template<typename FieldType>
    constexpr bool isSubField(int fid, FieldType& field)
    {
        return (fid == field.FID);
    }

    // toString
    template<int ...S>
    std::ostringstream& toStringUnwind(std::ostringstream& os, seq<S...>)
    {
        toString(os, std::get<S>(_fieldList) ...);
        return os;
    }

    template<typename FieldType, typename... FieldTypeList>
    std::ostringstream& toString(std::ostringstream& os, FieldType& field, FieldTypeList&... fieldList)
    {
        field.toString(os);
        return toString(os, fieldList...);
    }

    template<typename FieldType>
    std::ostringstream& toString(std::ostringstream& os, FieldType& field)
    {
        return field.toString(os);
    }

    // getSubField
    template<int ...S>
    bool getSubFieldUnwind(int fid, CachedField& retField, seq<S...>)
    {
        return getSubField(fid, retField, std::get<S>(_fieldList) ...);
    }

    template<typename FieldType, typename... FieldTypeList>
    typename std::enable_if<!FieldType::IS_GROUP, bool>::type
    getSubField(int fid, CachedField& retField, FieldType& field, FieldTypeList&... fieldList)
    {
        if(fid == field.FID)
        {
            // found
            std::cout << "getSubField: " << &field << " FID: " << (int) fid << std::endl;
            retField = field.get();
            return true;
        }

        return getSubField(fid, retField, fieldList...);
    }

    template<typename FieldType>
    typename std::enable_if<!FieldType::IS_GROUP, bool>::type
    getSubField(int fid, CachedField& retField, FieldType& field)
    {
        if(fid == field.FID)
        {
            // found
            std::cout << "getSubField: " << &field << " FID: " << (int) fid << std::endl;
            retField = field.get();
            return true;
        }
        else
        {
            // not found
            return false;
        }
    }

    template<typename FieldType, typename... FieldTypeList>
    typename std::enable_if<FieldType::IS_GROUP, bool>::type
    getSubField(int fid, CachedField& retField, FieldType& field, FieldTypeList&... fieldList)
    {
        return getSubField(fid, retField, fieldList...);
    }

    template<typename FieldType>
    typename std::enable_if<FieldType::IS_GROUP, bool>::type
    getSubField(int fid, CachedField& retField, FieldType& field)
    {
        return false;
    }

    // getSubGroup
    template<typename GroupType, int ...S>
    bool getSubGroupUnwind(int fid, GroupType& retField, seq<S...>)
    {
        return getSubGroup<GroupType, S...>(fid, retField, std::get<S>(_fieldList) ...);
    }

    template<typename GroupType, typename FieldType, typename... FieldTypeList>
    typename std::enable_if<FieldType::IS_GROUP, bool>::type
    getSubGroup(int fid, GroupType& retField, FieldType& field, FieldTypeList&... fieldList)
    {
        if(fid == field.FID)
        {
            // found
            std::cout << "getSubGroup: " << &field << " FID: " << (int) fid << std::endl;
            retField = field.get();
            return true;
        }

        return getSubGroup<GroupType, FieldTypeList...>(fid, retField, fieldList...);
    }

    template<typename GroupType, typename FieldType>
    typename std::enable_if<FieldType::IS_GROUP, bool>::type
    getSubGroup(int fid, GroupType& retField, FieldType& field)
    {
        if(fid == field.FID)
        {
            // found
            std::cout << "getSubGroup: " << &field << " FID: " << (int) fid << std::endl;
            retField = field.get();
            return true;
        }
        else
        {
            // not found
            return false;
        }
    }

    template<typename GroupType, typename FieldType, typename... FieldTypeList>
    typename std::enable_if<!FieldType::IS_GROUP, bool>::type
    getSubGroup(int fid, GroupType& retField, FieldType& field, FieldTypeList&... fieldList)
    {
        return getSubGroup<GroupType, FieldTypeList...>(fid, retField, fieldList...);
    }

    template<typename GroupType, typename FieldType>
    typename std::enable_if<!FieldType::IS_GROUP, bool>::type
    getSubGroup(int fid, GroupType& retField, FieldType& field)
    {
        return false;
    }

    // checkRequired
    bool checkRequired()
    {
        return checkRequiredUnwind(typename gens<sizeof...(FieldTypes)>::type());
    }

    template<int ...S>
    bool checkRequiredUnwind(seq<S...>)
    {
        return checkRequired(std::get<S>(_fieldList) ...);
    }

    template<typename FieldType, typename... FieldTypeList>
    bool checkRequired(FieldType& field, FieldTypeList&... fieldList)
    {
        if(field.IS_REQUIRED && !field.isSet())
        {
            // TODO - log ? Maybe a specific validation log setting
            return false;
        }
        return checkRequired(fieldList...);
    }

    template<typename FieldType>
    bool checkRequired(FieldType& field)
    {
        if(field.IS_REQUIRED && !field.isSet())
        {
            // TODO - log ? Maybe a specific validation log setting
            return false;
        }
        // else
        return true;
    }

    // clear
    template<int ...S>
    void clearUnwind(seq<S...>)
    {
        clear(std::get<S>(_fieldList) ...);
    }

    template<typename FieldType, typename... FieldTypeList>
    void clear(FieldType& field, FieldTypeList&... fieldList)
    {
        field.clear();
        clear(fieldList...);
    }

    template<typename FieldType>
    void clear(FieldType& field)
    {
        return field.clear();
    }


    HeaderType                  _header;
    TrailerType                 _trailer;
    std::tuple<FieldTypes...>   _fieldList;

    bool                        _strGenerated = false;
    char                        _outputStr[OUTPUT_STR_CAPACITY];
    size_t                      _outputStrLen = 0;

    bool                        _vecGenerated = false;
    size_t                      _countVec = 0;
    iovec                       _ioVec[VEC_CAPACITY]{};
};

template<const StringConstant& Name, const StringConstant& Type, typename Validate, typename HeaderType, typename TrailerType, typename... FieldTypes>
constexpr StringConstant SFIXMessage<Name, Type, Validate, HeaderType, TrailerType, FieldTypes...>::NAME;

template<const StringConstant& Name, const StringConstant& Type, typename Validate, typename HeaderType, typename TrailerType, typename... FieldTypes>
constexpr StringConstant SFIXMessage<Name, Type, Validate, HeaderType, TrailerType, FieldTypes...>::TYPE;

template<const StringConstant& Name, const StringConstant& Type, typename Validate, typename HeaderType, typename TrailerType, typename... FieldTypes>
constexpr bool SFIXMessage<Name, Type, Validate, HeaderType, TrailerType, FieldTypes...>::VALIDATE;

template<const StringConstant& Name, const StringConstant& Type, typename Validate, typename HeaderType, typename TrailerType, typename... FieldTypes>
constexpr size_t SFIXMessage<Name, Type, Validate, HeaderType, TrailerType, FieldTypes...>::VEC_CAPACITY;

template<const StringConstant& Name, const StringConstant& Type, typename Validate, typename HeaderType, typename TrailerType, typename... FieldTypes>
constexpr size_t SFIXMessage<Name, Type, Validate, HeaderType, TrailerType, FieldTypes...>::OUTPUT_STR_CAPACITY;

}  // namespace vf_fix



#endif /* SFIXMESSAGES_H_ */
