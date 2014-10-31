/*
 * SFIXFields.h
 *
 *  Created on: 29 May 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef SFIXFIELDS_H_
#define SFIXFIELDS_H_

#include <ostream>

#include "apps/fix_engine/dictionary/FIXField.h"
#include "apps/fix_engine/message/CachedField.h"
#include "utilities/StringConstant.h"

using namespace vf_common;

namespace vf_fix
{

template<int                        Fid,
         const StringConstant&      FidStr,
         const StringConstant&      Name,
         const StringConstant&      FieldTypeStr,
         const StringConstantArr&   EnumVal,
         const StringConstantArr&   DescVal,
         typename                   Required,
         typename                   Validate>
class SFIXField
{
public:
    constexpr static int                   FID         = Fid;
    constexpr static StringConstant        NAME        = Name;
    constexpr static StringConstant        FID_STR     = FidStr; // this is used for building FIX messages, e.g. set to '35='
    constexpr static StringConstant        TYPE_NAME   = FieldTypeStr;
    constexpr static StringConstantArr     ENUM_VAL    = EnumVal;
    constexpr static StringConstantArr     DESC_VAL    = DescVal;
    constexpr static bool                  IS_GROUP    = false;
    constexpr static bool                  IS_REQUIRED = Required::value;
    constexpr static bool                  VALIDATE    = Validate::value;
    constexpr static StringConstant        TRAILER     = StringConstant("\001\000");

    constexpr static FIXField::FieldType type()
    {
        return getType();
    }


    SFIXField()
    : _valIndex (-1)
    {}

    template<typename DecoderType>
    bool set(DecoderType& decoder)
    {
        if(UNLIKELY(!decoder.currentField().first))
        {
            return false;
        }

        // optional validation check
        if(!validate<VALIDATE, DecoderType>(decoder))
        {
            return false;
        }

        _value.setVal(decoder.currentField().second);
        return true;
    }

    bool isSet()
    {
        return _value.value() != NULL;
    }

    CachedField& get()
    {
        return _value;
    }

    template<typename ValType, typename ToString = std::true_type>
    void setValue(ValType& val)
    {
        _value.setCachedVal<ValType, ToString>(val);
    }

    template<size_t N>
    void setValue(const char (&val) [N])
    {
        _value.setCachedVal(val);
    }

    int index()
    {
        if(_valIndex > 0 || !_value.value())
        {
            return _valIndex;
        }

        if(EnumVal.size() > 0)
        {
            _valIndex = EnumVal.getIndex(_value.value());
        }

        return _valIndex;
    }

    const char * getDescription()
    {
        if(_value.value())
        {
            if(_valIndex < 0)
            {
                _valIndex = index();
            }

            if(_valIndex >= 0)
            {
                return DESC_VAL[_valIndex];
            }
            else
            {
                return NULL;
            }
        }

        return NULL;
    }

    void clear()
    {
        _valIndex = -1;
        _value.clear();
    }

    std::ostringstream& toString(std::ostringstream& os)
    {
        if(isSet())
        {
            os << FidStr << (_value.value() ? _value.value() : "0") << "|";
        }
        return os;
    }

    // TODO - need to optimise this function
    // write to the buffer passed in with a well formed fix field in the form of
    // <fix>=<val><SOH><NULL>
    bool getFIXStr(char * buffer, size_t size)
    {
        if(!buffer || !size || _value.empty())
        {
            return false;
        }

        int remainingLen = size;
        int len = strncpy(buffer, FID_STR, remainingLen);
        remainingLen -= len;
        buffer += len;
        if(remainingLen <= 0)
        {
            return false;
        }
        len = strncpy(buffer, _value.value(), remainingLen);
        remainingLen -= len;
        if(remainingLen <= 0)
        {
            return false;
        }
        buffer[len] = SOH;
        buffer[len+1] = '\0'; // good will

        return true;
    }

    // sets the pointer to the fix string into the iovec structure
    // the iovec structure passed in must be at least 3 vector wide
    bool setIoVec(iovec*& vec)
    {
        if(UNLIKELY((_value.empty() || !vec || !(vec+1) || !(vec+2))))
        {
            return false;
        }

        vec->iov_base = (void *) FID_STR.data();
        vec->iov_len = FID_STR.size();
        ++vec;

        vec->iov_base = (void *) _value.value();
        vec->iov_len = _value.size();
        ++vec;

        vec->iov_base = (void *) TRAILER.data();
        vec->iov_len = TRAILER.size();
        ++vec;

        return true;
    }

    bool setOutputBuffer(char *& buffer, int& remLen)
    {
        if(UNLIKELY((_value.empty())))
        {
            return false;
        }

        int lenRequired = FID_STR.size() + TRAILER.size() + _value.size();
        if(remLen < lenRequired)
        {
            return false;
        }

        // memcpy
        memcpy(buffer, FID_STR.data(), FID_STR.size());
        buffer += FID_STR.size();
        memcpy(buffer, _value.value(), _value.size());
        buffer += _value.size();
        memcpy(buffer, TRAILER.data(), TRAILER.size());
        buffer += TRAILER.size();

        return true;
    }

private:
    template<bool T, typename DecoderType>
    typename std::enable_if<T, bool>::type validate(DecoderType& decoder)
    {
        int fid = decoder.currentField().first;
        if(fid != FID)
        {
            return false;
        }

        if(getType() == FIXField::FieldType::UNKNOWN_TYPE)
        {
            return false;
        }

        // Might not need this check - we will check required fields in groups, collection and message level as well
        const char * val = decoder.currentField().second;
        if(IS_REQUIRED && !val)
        {
            return false;
        }

        if(EnumVal.size() > 0 && val && EnumVal.getIndex(val) < 0)
        {
            return false;
        }

        return true;
    }

    template<bool T, typename DecoderType>
    typename std::enable_if<!T, bool>::type validate(DecoderType& decoder)
    {
        return true;
    }

    constexpr static FIXField::FieldType getType()
    {
        return FieldTypeStr == StringConstant("INT") ? FIXField::FieldType::INT :
               FieldTypeStr == StringConstant("LENGTH") ? FIXField::FieldType::LENGTH :
               FieldTypeStr == StringConstant("TAGNUM") ? FIXField::FieldType::TAG_NUM :
               FieldTypeStr == StringConstant("SEQNUM") ? FIXField::FieldType::SEQ_NUM :
               FieldTypeStr == StringConstant("NUMINGROUP") ? FIXField::FieldType::NUM_IN_GROUP :
               FieldTypeStr == StringConstant("DAYOFMONTH") ? FIXField::FieldType::DAY_OF_MONTH :
               FieldTypeStr == StringConstant("FLOAT") ? FIXField::FieldType::FLOAT :
               FieldTypeStr == StringConstant("QTY") ? FIXField::FieldType::QTY :
               FieldTypeStr == StringConstant("PRICE") ? FIXField::FieldType::PRICE_OFFSET :
               FieldTypeStr == StringConstant("AMT") ? FIXField::FieldType::AMT :
               FieldTypeStr == StringConstant("PERCENTAGE") ? FIXField::FieldType::PERCENTAGE :
               FieldTypeStr == StringConstant("CHAR") ? FIXField::FieldType::CHAR :
               FieldTypeStr == StringConstant("BOOLEAN") ? FIXField::FieldType::BOOLEAN :
               FieldTypeStr == StringConstant("STRING") ? FIXField::FieldType::STRING :
               FieldTypeStr == StringConstant("MULTIPLECHARVALUE") ? FIXField::FieldType::MULTIPLE_CHAR_VALUE :
               FieldTypeStr == StringConstant("MULTIPLESTRINGVALUE") ? FIXField::FieldType::MULTIPLE_STRING_VALUE :
               FieldTypeStr == StringConstant("COUNTRY") ? FIXField::FieldType::COUNTRY :
               FieldTypeStr == StringConstant("CURRENCY") ? FIXField::FieldType::CURRENCY :
               FieldTypeStr == StringConstant("EXCHANGE") ? FIXField::FieldType::EXCHANGE :
               FieldTypeStr == StringConstant("MONTHYEAR") ? FIXField::FieldType::MONTH_YEAR :
               FieldTypeStr == StringConstant("UTCTIMESTAMP") ? FIXField::FieldType::UTC_TIMESTAMP :
               FieldTypeStr == StringConstant("UTCTIMEONLY") ? FIXField::FieldType::UTC_TIME_ONLY :
               FieldTypeStr == StringConstant("UTCDATEONLY") ? FIXField::FieldType::UTC_DATE_ONLY :
               FieldTypeStr == StringConstant("TIME") ? FIXField::FieldType::TIME :
               FieldTypeStr == StringConstant("DATE") ? FIXField::FieldType::DATE :
               FieldTypeStr == StringConstant("LOCALMKTDATE") ? FIXField::FieldType::LOCAL_MKT_DATE :
               FieldTypeStr == StringConstant("TZTIMEONLY") ? FIXField::FieldType::TZ_TIME_ONLY :
               FieldTypeStr == StringConstant("TZTIMESTAMP") ? FIXField::FieldType::TZ_TIMESTAMP :
               FieldTypeStr == StringConstant("DATA") ? FIXField::FieldType::DATA :
               FieldTypeStr == StringConstant("XMLDATA") ? FIXField::FieldType::XML_DATA :
               FieldTypeStr == StringConstant("LANGUAGE") ? FIXField::FieldType::LANGUAGE :
               FieldTypeStr == StringConstant("PATTERN") ? FIXField::FieldType::PATTERN :
               FieldTypeStr == StringConstant("TENOR") ? FIXField::FieldType::TENOR :
               FieldTypeStr == StringConstant("RESERVED100PLUS") ? FIXField::FieldType::RESERVED_100_PLUS :
               FieldTypeStr == StringConstant("RESERVED1000PLUS") ? FIXField::FieldType::RESERVED_1000_PLUS :
               FieldTypeStr == StringConstant("RESERVED4000PLUS") ? FIXField::FieldType::RESERVED_4000_PLUS :
               FieldTypeStr == StringConstant("COMPONENT") ? FIXField::FieldType::COMPONENT_TYPE :
               FIXField::FieldType::UNKNOWN_TYPE;
    }

    int             _valIndex;
    CachedField     _value;
};

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr int SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::FID;

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr StringConstant SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::FID_STR;

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr StringConstant SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::NAME;

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr StringConstant SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::TYPE_NAME;

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr bool SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::IS_REQUIRED;

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr bool SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::VALIDATE;

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr StringConstantArr SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::ENUM_VAL;

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr StringConstantArr SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::DESC_VAL;

template<int Fid, const StringConstant& FidStr, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename Required, typename Validate>
constexpr StringConstant SFIXField<Fid, FidStr, Name, FieldTypeStr, EnumVal, DescVal, Required, Validate>::TRAILER;

}  // namespace vf_fix



#endif /* SFIXFIELDS_H_ */
