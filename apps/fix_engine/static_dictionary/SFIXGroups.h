/*
 * SFIXGroups.h
 *
 *  Created on: 29 May 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef SFIXGROUPS_H_
#define SFIXGROUPS_H_

#include "SFIXFields.h"

namespace vf_fix
{
template<int                    Fid,
         const StringConstant&  Name,
         typename               Required,
         typename               Validate,
         unsigned int           Capacity,
         typename...            FieldTypes>
class SFIXGroup
{
public:
    SFIXGroup()
    : _numRepeating (0)
    {}

    constexpr static int                   FID         = Fid;
    constexpr static StringConstant        NAME        = Name;
    constexpr static StringConstant        TYPE_NAME   = StringConstant("NUMINGROUP");
    constexpr static FIXField::FieldType   TYPE        = FIXField::FieldType::NUM_IN_GROUP;
    constexpr static bool                  IS_GROUP    = true;
    constexpr static bool                  IS_REQUIRED = Required::value;
    constexpr static bool                  VALIDATE    = Validate::value;
    constexpr static unsigned int          CAPACITY    = Capacity;

    constexpr static FIXField::FieldType type()
    {
        return TYPE;
    }

    template<typename DecoderType>
    bool set(DecoderType& decoder)
    {
        if(UNLIKELY(!decoder.currentField().first || !decoder.currentField().second))
        {
            return false;
        }

        int fid = atoi(decoder.currentField().first);
        if(fid != FID)
        {
            return false;
        }

        _numRepeating = atoi(decoder.currentField().second);
        int numRepeats = _numRepeating;
        int lastRepeatingFid = 0;
        int firstRepeatingFid = 0;
        int fieldGroupIndex = 0;

        while(numRepeats > 0)
        {
            if(!decoder.next())
            {
                // this may be valid if this is the last/only repeating group
                if(_numRepeating == 1)
                {
                    // this is to catch instances where there is only 1 repeating group
                    decoder.rewind();
                    return (firstRepeatingFid != 0);
                }
                return false;
            }

            // TODO - validate fields ?

            auto& subField = decoder.currentField();
            int fid = atoi(subField.first);

            if(_numRepeating == 1 && !isSubField(fid))
            {
                // this is to catch instances where there is only 1 repeating group
                decoder.rewind();
                return (firstRepeatingFid != 0);
            }

            if(!firstRepeatingFid)
            {
                firstRepeatingFid = fid;
            }
            else if(firstRepeatingFid == fid)
            {
                if(!fieldGroupIndex)
                {
                    --numRepeats; // catch end of first repeating group
                }
                ++fieldGroupIndex;
            }

            if(!setSubField(decoder, fid, fieldGroupIndex))
            {
                // error
                return false;
            }

            if(!fieldGroupIndex) // first repeating group
            {
                lastRepeatingFid = fid;
            }
            else if (lastRepeatingFid == fid)
            {
                --numRepeats;
            }
        }

        return true;
    }

    int getNumRepeating()
    {
        return _numRepeating;
    }

    template<typename DecoderType>
    bool setSubField(DecoderType& decoder, int fid, int index)
    {
        if(index >= _numRepeating)
        {
            return false;
        }

        return setSubFieldUnwind(decoder, fid, index, typename gens<sizeof...(FieldTypes)>::type());
    }

    constexpr bool isSubField(int fid)
    {
        return isSubFieldUnwind(fid, typename gens<sizeof...(FieldTypes)>::type());
    }

    bool getSubField(int fid, int index, CachedField& retField)
    {
        if(index >= _numRepeating)
        {
            return false;
        }

        return getSubFieldUnwind(fid, index, retField, typename gens<sizeof...(FieldTypes)>::type());
    }

    std::ostringstream& toString(std::ostringstream& os)
    {
        return toStringUnwind(os, typename gens<sizeof...(FieldTypes)>::type());
    }

private:

    // setSubField
    template<typename DecoderType, int ...S>
    bool setSubFieldUnwind(DecoderType& decoder, int fid, int index, seq<S...>)
    {
        return setSubField(decoder, fid, std::get<S>(_fieldList[index]) ...);
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

    // isSubField
    template<int ...S>
    constexpr bool isSubFieldUnwind(int fid, seq<S...>)
    {
        return isSubField(fid, std::get<S>(_fieldList[0]) ...);
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

    // getSubField
    template<int ...S>
    bool getSubFieldUnwind(int fid, int index, CachedField& retField, seq<S...>)
    {
        return getSubField(fid, retField, std::get<S>(_fieldList[index]) ...);
    }

    template<typename FieldType, typename... FieldTypeList>
    bool getSubField(int fid, CachedField& retField, FieldType& field, FieldTypeList&... fieldList)
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
    bool getSubField(int fid, CachedField& retField, FieldType& field)
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

    // toString
    template<int ...S>
    std::ostringstream& toStringUnwind(std::ostringstream& os, seq<S...>)
    {
        for(int index=0;index<_numRepeating;++index)
        {
            toString(os, std::get<S>(_fieldList[index]) ...);
        }
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

    int                         _numRepeating;
    std::tuple<FieldTypes...>   _fieldList[Capacity];

};

template<int Fid, const StringConstant& Name, typename Required, typename Validate, unsigned int Capacity, typename... FieldTypes>
constexpr int SFIXGroup<Fid, Name, Required, Validate, Capacity, FieldTypes...>::FID;

template<int Fid, const StringConstant& Name, typename Required, typename Validate, unsigned int Capacity, typename... FieldTypes>
constexpr StringConstant SFIXGroup<Fid, Name, Required, Validate, Capacity, FieldTypes...>::NAME;

template<int Fid, const StringConstant& Name, typename Required, typename Validate, unsigned int Capacity, typename... FieldTypes>
constexpr StringConstant SFIXGroup<Fid, Name, Required, Validate, Capacity, FieldTypes...>::TYPE_NAME;

template<int Fid, const StringConstant& Name, typename Required, typename Validate, unsigned int Capacity, typename... FieldTypes>
constexpr FIXField::FieldType SFIXGroup<Fid, Name, Required, Validate, Capacity, FieldTypes...>::TYPE;

template<int Fid, const StringConstant& Name, typename Required, typename Validate, unsigned int Capacity, typename... FieldTypes>
constexpr bool SFIXGroup<Fid, Name, Required, Validate, Capacity, FieldTypes...>::IS_GROUP;

template<int Fid, const StringConstant& Name, typename Required, typename Validate, unsigned int Capacity, typename... FieldTypes>
constexpr bool SFIXGroup<Fid, Name, Required, Validate, Capacity, FieldTypes...>::IS_REQUIRED;

template<int Fid, const StringConstant& Name, typename Required, typename Validate, unsigned int Capacity, typename... FieldTypes>
constexpr bool SFIXGroup<Fid, Name, Required, Validate, Capacity, FieldTypes...>::VALIDATE;

template<int Fid, const StringConstant& Name, typename Required, typename Validate, unsigned int Capacity, typename... FieldTypes>
constexpr unsigned int SFIXGroup<Fid, Name, Required, Validate, Capacity, FieldTypes...>::CAPACITY;


/*
class SFIXGroup<753, Traits, GroupCapacity>
{
public:
    SFIXGroup()
    : _numRepeating (0)
    {}

    constexpr static int                   FID         = 753;
    constexpr static const char *          NAME        = "NoPosAmt";
    constexpr static FIXField::FieldType   TYPE        = FIXField::FieldType::NUM_IN_GROUP;
    constexpr static const char *          TYPE_NAME   = "NUMINGROUP";
    constexpr static bool                  IS_GROUP    = true;
    constexpr static bool                  IS_REQUIRED = Traits::Required::value;
    constexpr static bool                  VALIDATE    = Traits::ValidateParse::value;

    typedef FIXFieldTraits<std::true_type,  typename Traits::ValidateParse>   RequiredFieldTraits;
    typedef FIXFieldTraits<std::false_type, typename Traits::ValidateParse>   OptionalFieldTraits;

    template<typename DecoderType>
    bool set(DecoderType& decoder)
    {
        if(UNLIKELY(!decoder.currentField().first || !decoder.currentField().second))
        {
            return false;
        }

        int fid = atoi(decoder.currentField().first);
        if(fid != FID)
        {
            return false;
        }

        _numRepeating = atoi(decoder.currentField().second);
        int numRepeats = _numRepeating;
        int lastRepeatingFid = 0;
        int firstRepeatingFid = 0;
        int fieldGroupIndex = 0;

        while(numRepeats > 0)
        {
            if(!decoder.next())
            {
                // this may be valid if this is the last/only repeating group
                if(_numRepeating == 1)
                {
                    // this is to catch instances where there is only 1 repeating group
                    decoder.rewind();
                    return (firstRepeatingFid != 0);
                }
                return false;
            }

            // TODO - validate fields ?

            auto& subField = decoder.currentField();
            int fid = atoi(subField.first);

            if(_numRepeating == 1 && !isSubField(fid))
            {
                // this is to catch instances where there is only 1 repeating group
                decoder.rewind();
                return (firstRepeatingFid != 0);
            }

            if(!firstRepeatingFid)
            {
                firstRepeatingFid = fid;
            }
            else if(firstRepeatingFid == fid)
            {
                if(!fieldGroupIndex)
                {
                    --numRepeats; // catch end of first repeating group
                }
                ++fieldGroupIndex;
            }

            if(!setSubField(decoder, fid, fieldGroupIndex))
            {
                // error
                return false;
            }

            if(!fieldGroupIndex) // first repeating group
            {
                lastRepeatingFid = fid;
            }
            else if (lastRepeatingFid == fid)
            {
                --numRepeats;
            }
        }

        return true;
    }

    int getNumRepeating()
    {
        return _numRepeating;
    }

    template<int FID>
    typename std::enable_if<FID==707, SFIXField<707, OptionalFieldTraits>&>::type getSubField(int index)
    {
        return _field707Array[index];
    }

    template<int FID>
    typename std::enable_if<FID==708, SFIXField<708, OptionalFieldTraits>&>::type getSubField(int index)
    {
        return _field708Array[index];
    }

    template<int FID>
    typename std::enable_if<FID==1055, SFIXField<1055, OptionalFieldTraits>&>::type getSubField(int index)
    {
        return _field1055Array[index];
    }

    bool isSubField(int fid)
    {
        return (fid == 707 || fid == 708 || fid == 1055);
    }

    std::ostringstream& toString(std::ostringstream& os)
    {
        os << "Group FID: " << FID << std::endl;
        for(int count=0;count<GroupCapacity;++count)
        {
            if(!_field707Array[count].get().empty())
            {
                _field707Array[count].toString(os);
            }
            if(!_field708Array[count].get().empty())
            {
                _field708Array[count].toString(os);
            }
            if(!_field1055Array[count].get().empty())
            {
                _field1055Array[count].toString(os);
            }
        }
        return os;
    }

private:

    template<typename DecoderType>
    bool setSubField(DecoderType& decoder, int fid, int index)
    {
        // TODO - need to make this more efficient
        switch(fid)
        {
        case 707:
            return getSubField<707>(index).set(decoder);
        case 708:
            return getSubField<708>(index).set(decoder);
        case 1055:
            return getSubField<1055>(index).set(decoder);
        }
    }

    int  _numRepeating;

    std::array<SFIXField<707,   OptionalFieldTraits>, GroupCapacity> _field707Array;
    std::array<SFIXField<708,   OptionalFieldTraits>, GroupCapacity> _field708Array;
    std::array<SFIXField<1055,  OptionalFieldTraits>, GroupCapacity> _field1055Array;

};

*/

} // namespace vf_fix

#endif /* SFIXGROUPS_H_ */
