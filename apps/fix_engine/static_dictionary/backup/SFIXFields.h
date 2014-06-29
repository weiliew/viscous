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

namespace vf_fix
{

// TODO - can we specialise on string here ?

template<typename IsRequired, typename Validate, size_t MaxGroupSize>
struct FIXFieldTraits
{
    typedef IsRequired      Required;
    typedef Validate        ValidateParse;
    constexpr size_t        GroupCapacity = MaxGroupSize;
};

template<unsigned int, typename Traits>
class SFIXField
{
};

template<typename Traits>
class SFIXField<753>
{
public:
    typedef typename DecoderType::FieldPairType     FieldPairType;

    SFIXField()
    : _numRepeating (0)
    {}


    constexpr int                   FID         = 753;
    constexpr const char *          NAME        = "NoPosAmt";
    constexpr FIXField::FieldType   TYPE        = FIXField::FieldType::NUM_IN_GROUP;
    constexpr const char *          TYPE_NAME   = "NUMINGROUP";
    constexpr bool                  IS_GROUP    = true;
    constexpr bool                  IS_REQUIRED = Traits::Required::value;
    constexpr bool                  VALIDATE    = Traits::ValidateParse::value;

    typedef FIXFieldTraits<std::true_type,  VALIDATE>   RequiredFieldTraits;
    typedef FIXFieldTraits<std::false_type, VALIDATE>   OptionalFieldTraits;

    constexpr const char * getDescFromEnum();
    // TODO - more
    // enum map
    // if group - list of group element

    template<int FID>
    SFIXField* getSubField()
    {
        return NULL;
    }

    template<>
    SFIXField* getSubField<707>()
    {
        return &_subFields[0];
    }

    template<>
    SFIXField* getSubField<708>()
    {
        return &_subFields[1];
    }

    template<>
    SFIXField* getSubField<1055>()
    {
        return &_subFields[2];
    }

    template<typename DecoderType>
    size_t parseData(DecoderType& decoder)
    {
        // TODO - how to report parse error ? exceptions ?
        // data must be in the format <fid><NULL><num repeating><NULL><optional><NULL><val>.....
        // returns bytes consumed

        if(UNLIKELY(!decoder.next()))
        {
            return false;
        }

        // optional validation check
        if(!validate<VALIDATE, DecoderType>(decoder))
        {
            return 0; // or report error
        }

        FieldPairType& fieldPair = decoder.currentField();
        _numRepeating = atoi(fieldPair.second);
        int numRepeats = _numRepeating;
        int lastRepeatingFid = 0;
        int firstRepeatingFid = 0;
        bool inFirstGroup = true;
        while(numRepeats > 0)
        {
            if(UNLIKELY(!decoder.next()))
            {
                return false;
            }

            FieldPairType& subField = decoder.currentField();
            int fid = atoi(subField.first);
            if(!firstRepeatingFid)
            {
                firstRepeatingFid = fid;
            }
            else if(firstRepeatingFid == fid)
            {
                inFirstGroup = false;
            }

            if(inFirstGroup)
            {
                lastRepeatingFid = fid;
            }
            else if (lastRepeatingFid == fid)
            {
                --numRepeats;
            }

            SFIXField* field = getSubField<fid>();
            if(!field)
            {
                // rewind as we possibly have reached the end of the group
                decoder.prev();
                if(numRepeats)
                {
                    // error
                    return false;
                }
                break;
            }
            field->setVal();
        }
    }

    int getNumRepeating()
    {
        return _numRepeating;
    }

    // begin and end - MPL ??
    // for each - MPL ??

    // get set value - will not have one if this is a group

private:
    template<typename T, typename DecoderType>
    typename std::enable_if<T::value, bool>::type validate(DecoderType& decoder)
    {
        if(UNLIKELY(!decoder.currentField().first))
        {
            return false;
        }

        int fid = atoi(decoder.currentField().first);
        if(fid != FID)
        {
            if(IS_REQUIRED)
            {
                return false;
            }
            return true; // optional field - not an error
        }
        return true;
    }

    template<typename T, typename DecoderType>
    typename std::enable_if<!T::value, bool>::type validate(DecoderType& decoder)
    {
        return true;
    }

    // TODO - how to handle num repeating fields ?
    int  _numRepeating;

    struct FieldGroup
    {
        SFIXField<707,  OptionalFieldTraits> field1;
    };
    typedef std::array<SFIXField, 3> FieldGroup;
    constexpr std::array<FielgFroup, Traits::GroupCapacity> _repeatingGroupFields;

    constexpr std::array<SFIXField, 3>    _subFields = {SFIXField<707,  OptionalFieldTraits>,
                                                        SFIXField<708,  OptionalFieldTraits>,
                                                        SFIXField<1055, OptionalFieldTraits>};
};

template<typename Traits>
class SFIXField<707>
{
public:
    SFIXField()
    : valIndex_ (-1)
    , valStr_ (NULL)
    , _isSet(false)
    , _isParsed(false)
    {}

    constexpr unsigned int          FID         = 707;
    constexpr const char *          NAME        = "PosAmtType";
    constexpr FIXField::FieldType   TYPE        = FIXField::FieldType::STRING;
    constexpr const char *          TYPE_NAME   = "STRING";
    constexpr bool                  IS_GROUP    = false;
    constexpr bool                  IS_REQUIRED = Traits::Required::value;
    constexpr bool                  VALIDATE    = Traits::ValidateParse::value;

    constexpr const char * getDescFromEnum();
    // TODO - more
    // enum map
    // if group - list of group element


    // begin and end - MPL ??
    // for each - MPL ??

    // get set value - will not have one if this is a group

    void set(const char * val)
    {
        _valStr = val;
        if(VALIDATE)
        {
            // TODO - string match the value
            _isParsed = true;
        }
        _isSet = true;
    }

    bool isSet()
    {
        return _isSet;
    }

    bool isParsed()
    {
        return _isParsed;
    }

private:
    constexpr size_t _numEnum_= 9;
    constexpr const char *  _valEnum[_numEnum_] = {"CASH", "CRES", "FMTM", "IMTM", "PREM", "SMTM", "TVAR", "VADJ", "SETL"};
    constexpr const char *  _valDesc[_numEnum_] = {"CASH_AMOUNT", "CASH_RESIDUAL_AMOUNT", "FINAL_MARK_TO_MARKET_AMOUNT",
            "INCREMENTAL_MARK_TO_MARKET_AMOUNT", "PREMIUM_AMOUNT", "START_OF_DAY_MARK_TO_MARKET_AMOUNT", "TRADE_VARIATION_AMOUNT",
            "VALUE_ADJUSTED_AMOUNT", "SETTLEMENT_VALUE"};

    int             _valIndex;
    const char *    _valStr;
    bool            _isSet;
    bool            _isParsed;

    // lookup from string val to index - string hash ??
};


template<typename Traits>
class SFIXField<708>
{
public:
    SFIXField()
    : valIndex_ (-1)
    , valStr_ (NULL)
    {}

    constexpr unsigned int          FID         = 708;
    constexpr const char *          NAME        = "PosAmt";
    constexpr FIXField::FieldType   TYPE        = FIXField::FieldType::AMT;
    constexpr const char *          TYPE_NAME   = "AMT";
    constexpr bool                  IS_GROUP    = false;
    constexpr bool                  IS_REQUIRED = Traits::Required::value;
    constexpr bool                  VALIDATE    = Traits::ValidateParse::value;

    constexpr const char * getDescFromEnum()
    {
        return "PosAmt";
    }

    // TODO - more

private:
    constexpr size_t    _numEnum_= 0;
    unsigned int        _valAmount;
    const char *        _valStr;
};

template<typename Traits>
class SFIXField<1055>
{
public:
    SFIXField()
    : valIndex_ (-1)
    , valStr_ (NULL)
    {}

    constexpr unsigned int          FID         = 1055;
    constexpr const char *          NAME        = "PositionCurrency";
    constexpr FIXField::FieldType   TYPE        = FIXField::FieldType::STRING;
    constexpr const char *          TYPE_NAME   = "STRING";
    constexpr bool                  IS_GROUP    = false;
    constexpr bool                  IS_REQUIRED = Traits::Required::value;
    constexpr bool                  VALIDATE    = Traits::ValidateParse::value;

    constexpr const char * getDescFromEnum()
    {
        return "PosAmt";
    }

    // TODO - more

private:
    constexpr size_t    _numEnum_= 0;
    const char *        _valStr;
};




}  // namespace vf_fix



#endif /* SFIXFIELDS_H_ */
