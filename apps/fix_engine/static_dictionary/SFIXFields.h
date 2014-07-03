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
         const StringConstant&      Name,
         const StringConstant&      FieldTypeStr,
         const StringConstantArr&   EnumVal,
         const StringConstantArr&   DescVal,
         typename                   IsGroup,
         typename                   Required,
         typename                   Validate>
class SFIXField
{
public:
    constexpr static int                   FID         = Fid;
    constexpr static StringConstant        NAME        = Name;
    constexpr static StringConstant        TYPE_NAME   = FieldTypeStr;
    constexpr static StringConstantArr     ENUM_VAL    = EnumVal;
    constexpr static StringConstantArr     DESC_VAL    = DescVal;
    constexpr static bool                  IS_GROUP    = IsGroup::value;
    constexpr static bool                  IS_REQUIRED = Required::value;
    constexpr static bool                  VALIDATE    = Validate::value;

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

    std::ostringstream& toString(std::ostringstream& os)
    {
        if(isSet())
        {
            os << (int) FID << "=" << (_value.value() ? _value.value() : "0") << "|";
        }
        return os;
    }

private:
    template<bool T, typename DecoderType>
    typename std::enable_if<T, bool>::type validate(DecoderType& decoder)
    {
        int fid = atoi(decoder.currentField().first);
        if(fid != FID)
        {
            return false;
        }

        if(!decoder.currentField().second)
        {
            return false;
        }

        if(getType() == FIXField::FieldType::UNKNOWN_TYPE)
        {
            return false;
        }

        if(IS_REQUIRED && !_value.value())
        {
            return false;
        }

        const char * val = decoder.currentField().second;
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

template<int Fid, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename IsGroup, typename Required, typename Validate>
constexpr int SFIXField<Fid, Name, FieldTypeStr, EnumVal, DescVal, IsGroup, Required, Validate>::FID;

template<int Fid, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename IsGroup, typename Required, typename Validate>
constexpr StringConstant SFIXField<Fid, Name, FieldTypeStr, EnumVal, DescVal, IsGroup, Required, Validate>::NAME;

template<int Fid, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename IsGroup, typename Required, typename Validate>
constexpr StringConstant SFIXField<Fid, Name, FieldTypeStr, EnumVal, DescVal, IsGroup, Required, Validate>::TYPE_NAME;

template<int Fid, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename IsGroup, typename Required, typename Validate>
constexpr bool SFIXField<Fid, Name, FieldTypeStr, EnumVal, DescVal, IsGroup, Required, Validate>::IS_GROUP;

template<int Fid, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename IsGroup, typename Required, typename Validate>
constexpr bool SFIXField<Fid, Name, FieldTypeStr, EnumVal, DescVal, IsGroup, Required, Validate>::IS_REQUIRED;

template<int Fid, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename IsGroup, typename Required, typename Validate>
constexpr bool SFIXField<Fid, Name, FieldTypeStr, EnumVal, DescVal, IsGroup, Required, Validate>::VALIDATE;

template<int Fid, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename IsGroup, typename Required, typename Validate>
constexpr StringConstantArr SFIXField<Fid, Name, FieldTypeStr, EnumVal, DescVal, IsGroup, Required, Validate>::ENUM_VAL;

template<int Fid, const StringConstant& Name, const StringConstant& FieldTypeStr, const StringConstantArr& EnumVal, const StringConstantArr& DescVal, typename IsGroup, typename Required, typename Validate>
constexpr StringConstantArr SFIXField<Fid, Name, FieldTypeStr, EnumVal, DescVal, IsGroup, Required, Validate>::DESC_VAL;

/* To be removed ...

template<typename Traits>
class SFIXField<707, Traits>
{
public:
    SFIXField()
    : _valIndex (-1)
    {}

    constexpr static unsigned int          FID         = 707;
    constexpr static const char *          NAME        = "PosAmtType";
    constexpr static FIXField::FieldType   TYPE        = FIXField::FieldType::STRING;
    constexpr static const char *          TYPE_NAME   = "STRING";
    constexpr static bool                  IS_GROUP    = false;
    constexpr static bool                  IS_REQUIRED = Traits::Required::value;
    constexpr static bool                  VALIDATE    = Traits::ValidateParse::value;

    constexpr static size_t _numEnum = 9;
    constexpr static const char *  _valEnum[_numEnum] = {"CASH", "CRES", "FMTM", "IMTM", "PREM", "SMTM", "TVAR", "VADJ", "SETL"};
    constexpr static const char *  _valDesc[_numEnum] = {"CASH_AMOUNT", "CASH_RESIDUAL_AMOUNT", "FINAL_MARK_TO_MARKET_AMOUNT",
            "INCREMENTAL_MARK_TO_MARKET_AMOUNT", "PREMIUM_AMOUNT", "START_OF_DAY_MARK_TO_MARKET_AMOUNT", "TRADE_VARIATION_AMOUNT",
            "VALUE_ADJUSTED_AMOUNT", "SETTLEMENT_VALUE"};

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

    int index(const char * value)
    {
        // TODO - make this more efficient
        for(int i=0;i<_numEnum;++i)
        {
            if(!strcmp(value, _valEnum[i]))
            {
                // found
                return i;
            }
        }

        return -1;
    }

    const char * getDescription()
    {
        if(_value.value())
        {
            if(_valIndex < 0)
            {
                _valIndex = index(_value.value());
            }

            if(_valIndex >= 0)
            {
                return _valDesc[_valIndex];
            }
            else
            {
                return NULL;
            }
        }

        return NULL;
    }

    std::ostringstream& toString(std::ostringstream& os)
    {
        os << (unsigned int) FID << "=" << _value.value() << "|";
        return os;
    }

private:
    template<bool T, typename DecoderType>
    typename std::enable_if<T, bool>::type validate(DecoderType& decoder)
    {
        int fid = atoi(decoder.currentField().first);
        if(fid != FID)
        {
            return false;
        }

        if(!decoder.currentField().second)
        {
            return false;
        }

        _valIndex = index(decoder.currentField().second);
        if(_valIndex < 0)
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

    int             _valIndex;
    CachedField     _value;
};

template<typename Traits> constexpr unsigned int          SFIXField<707, Traits>::FID;
template<typename Traits> constexpr const char *          SFIXField<707, Traits>::NAME;
template<typename Traits> constexpr FIXField::FieldType   SFIXField<707, Traits>::TYPE;
template<typename Traits> constexpr const char *          SFIXField<707, Traits>::TYPE_NAME;
template<typename Traits> constexpr bool                  SFIXField<707, Traits>::IS_GROUP;
template<typename Traits> constexpr bool                  SFIXField<707, Traits>::IS_REQUIRED;
template<typename Traits> constexpr bool                  SFIXField<707, Traits>::VALIDATE;
template<typename Traits> constexpr size_t                SFIXField<707, Traits>::_numEnum;
template<typename Traits> constexpr const char *          SFIXField<707, Traits>::_valEnum[_numEnum];
template<typename Traits> constexpr const char *          SFIXField<707, Traits>::_valDesc[_numEnum];

template<typename Traits>
class SFIXField<708, Traits>
{
public:
    SFIXField()
    {}

    constexpr static unsigned int          FID         = 708;
    constexpr static const char *          NAME        = "PosAmt";
    constexpr static FIXField::FieldType   TYPE        = FIXField::FieldType::AMT;
    constexpr static const char *          TYPE_NAME   = "AMT";
    constexpr static bool                  IS_GROUP    = false;
    constexpr static bool                  IS_REQUIRED = Traits::Required::value;
    constexpr static bool                  VALIDATE    = Traits::ValidateParse::value;

    template<typename DecoderType>
    bool set(DecoderType& decoder)
    {
        if(UNLIKELY(!decoder.currentField().first))
        {
            return false;
        }

        int fid = atoi(decoder.currentField().first);
        if(fid != FID)
        {
            // not for this field
            return false;
        }

        // optional validation check
        if(!validate<typename Traits::ValidateParse, DecoderType>(decoder))
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

    int index(const char * value)
    {
        return -1;
    }

    const char * getDescription()
    {
        return NULL;
    }

    std::ostringstream& toString(std::ostringstream& os)
    {
        os << (unsigned int) FID << "=" << _value.value() << "|";
        return os;
    }

private:
    template<typename T, typename DecoderType>
    typename std::enable_if<T::value, bool>::type validate(DecoderType& decoder)
    {
        if(!decoder.currentField().second)
        {
            return false;
        }

        return true;
    }

    template<typename T, typename DecoderType>
    typename std::enable_if<!T::value, bool>::type validate(DecoderType& decoder)
    {
        return true;
    }

    CachedField     _value;
};

template<typename Traits> constexpr unsigned int          SFIXField<708, Traits>::FID;
template<typename Traits> constexpr const char *          SFIXField<708, Traits>::NAME;
template<typename Traits> constexpr FIXField::FieldType   SFIXField<708, Traits>::TYPE;
template<typename Traits> constexpr const char *          SFIXField<708, Traits>::TYPE_NAME;
template<typename Traits> constexpr bool                  SFIXField<708, Traits>::IS_GROUP;
template<typename Traits> constexpr bool                  SFIXField<708, Traits>::IS_REQUIRED;
template<typename Traits> constexpr bool                  SFIXField<708, Traits>::VALIDATE;

template<typename Traits>
class SFIXField<1055, Traits>
{
public:
    SFIXField()
    {}

    constexpr static unsigned int          FID         = 1055;
    constexpr static const char *          NAME        = "PositionCurrency";
    constexpr static FIXField::FieldType   TYPE        = FIXField::FieldType::STRING;
    constexpr static bool                  IS_GROUP    = false;
    constexpr static bool                  IS_REQUIRED = Traits::Required::value;
    constexpr static bool                  VALIDATE    = Traits::ValidateParse::value;

    template<typename DecoderType>
    bool set(DecoderType& decoder)
    {
        if(UNLIKELY(!decoder.currentField().first))
        {
            return false;
        }

        int fid = atoi(decoder.currentField().first);
        if(fid != FID)
        {
            // not for this field
            return false;
        }

        // optional validation check
        if(!validate<typename Traits::ValidateParse, DecoderType>(decoder))
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

    int index(const char * value)
    {
        return -1;
    }

    const char * getDescription()
    {
        return NULL;
    }

    std::ostringstream& toString(std::ostringstream& os)
    {
        os << (unsigned int) FID << "=" << _value.value() << "|";
        return os;
    }

private:
    template<typename T, typename DecoderType>
    typename std::enable_if<T::value, bool>::type validate(DecoderType& decoder)
    {
        if(!decoder.currentField().second)
        {
            return false;
        }

        return true;
    }

    template<typename T, typename DecoderType>
    typename std::enable_if<!T::value, bool>::type validate(DecoderType& decoder)
    {
        return true;
    }

    CachedField     _value;
};

template<typename Traits> constexpr unsigned int          SFIXField<1055, Traits>::FID;
template<typename Traits> constexpr const char *          SFIXField<1055, Traits>::NAME;
template<typename Traits> constexpr FIXField::FieldType   SFIXField<1055, Traits>::TYPE;
template<typename Traits> constexpr const char *          SFIXField<1055, Traits>::TYPE_NAME;
template<typename Traits> constexpr bool                  SFIXField<1055, Traits>::IS_GROUP;
template<typename Traits> constexpr bool                  SFIXField<1055, Traits>::IS_REQUIRED;
template<typename Traits> constexpr bool                  SFIXField<1055, Traits>::VALIDATE;
*/

}  // namespace vf_fix



#endif /* SFIXFIELDS_H_ */
