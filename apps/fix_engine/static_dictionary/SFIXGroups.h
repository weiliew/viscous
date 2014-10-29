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
    constexpr static size_t                VEC_CAPACITY = sizeof...(FieldTypes)*3*CAPACITY;

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

        if(decoder.currentField().first != FID)
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

            auto& subField = decoder.currentField();
            int fid = subField.first;

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

        // optional validation check
        if(!validate<VALIDATE>())
        {
            return false;
        }

        return true;
    }

    template<typename ValType>
    void setValue(ValType&)
    {
        // no-op on setValue for a group field type
    }

    bool isSet()
    {
        return _numRepeating>0;
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

    template<typename GroupType>
    bool getSubGroup(int fid, int index, GroupType& retField)
    {
        return getSubGroupUnwind(fid, index, retField, typename gens<sizeof...(FieldTypes)>::type());
    }

    void clear()
    {
        clearUnwind();
    }

    std::ostringstream& toString(std::ostringstream& os)
    {
        return toStringUnwind(os, typename gens<sizeof...(FieldTypes)>::type());
    }

    // sets the iovec structure passed in
    bool setIoVec(iovec*& vec)
    {
        return setIoVecUnwind(vec, typename gens<sizeof...(FieldTypes)>::type());
    }

    bool setOutputBuffer(char *& buffer, int& remLen)
    {
        return setOutputBufferUnwind(buffer, remLen, typename gens<sizeof...(FieldTypes)>::type());
    }

private:
    template<bool T>
    typename std::enable_if<T, bool>::type validate()
    {
        return checkRequired();
    }

    template<bool T>
    typename std::enable_if<!T, bool>::type validate()
    {
        return true;
    }

    // setIoVec
    template<int ...S>
    bool setIoVecUnwind(iovec*& vec, seq<S...>)
    {
        for(int index=0;index<_numRepeating;++index)
        {
            if(!setIoVec(vec, std::get<S>(_fieldList[index]) ...))
            {
                return false;
            }
        }

        return true;
    }

    template<typename FieldType, typename... FieldTypeList>
    bool setIoVec(iovec*& vec, FieldType& field, FieldTypeList&... fieldList)
    {
        if(field.isSet())
        {
            if(!field.setIoVec(vec))
            {
                return false;
            }
        }
        return setIoVec(vec, fieldList...);
    }

    template<typename FieldType>
    bool setIoVec(iovec*& vec, FieldType& field)
    {
        if(field.isSet())
        {
            if(!field.setIoVec(vec))
            {
                return false;
            }
        }

        return true;
    }

    // setOutputBuffer
    template<int ...S>
    bool setOutputBufferUnwind(char *& buffer, int& remLen, seq<S...>)
    {
        for(int index=0;index<_numRepeating;++index)
        {
            if(!setOutputBuffer(buffer, remLen, std::get<S>(_fieldList[index]) ...))
            {
                return false;
            }
        }

        return true;
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

    // getSubGroup
    template<typename GroupType, int ...S>
    bool getSubGroupUnwind(int fid, int index, GroupType& retField, seq<S...>)
    {
        return getSubGroup<GroupType, S...>(fid, retField, std::get<S>(_fieldList[index]) ...);
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

    // clear
    template<int ...S>
    void clearUnwind(seq<S...>)
    {
        for(int index=0;index<_numRepeating;++index)
        {
            clear(std::get<S>(_fieldList[index]) ...);
        }
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

    // checkRequired
    bool checkRequired()
    {
        return checkRequiredUnwind(typename gens<sizeof...(FieldTypes)>::type());
    }

    template<int ...S>
    bool checkRequiredUnwind(seq<S...>)
    {
        for(int index=0;index<_numRepeating;++index)
        {
            if(!checkRequired(std::get<S>(_fieldList[index]) ...))
            {
                // TODO - log ?
                return false;
            }
        }
        return true;
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

} // namespace vf_fix

#endif /* SFIXGROUPS_H_ */
