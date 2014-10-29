/*
 * SFIXCollections.h
 *
 *  Created on: 22 Jul 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef SFIXCOLLECTIONS_H_
#define SFIXCOLLECTIONS_H_


#include "SFIXFields.h"

namespace vf_fix
{

// this class is used for storing header and trailer of a message - it is similar to a group but is non repeating

template<typename      Validate,
         typename...   FieldTypes>
class SFIXCollection
{
public:
    SFIXCollection()
    {}

    constexpr static int                   FID          = 0;
    constexpr static StringConstant        NAME         = StringConstant("collections");
    constexpr static bool                  IS_GROUP     = false;
    constexpr static bool                  IS_REQUIRED  = true;
    constexpr static bool                  VALIDATE     = Validate::value;
    constexpr static size_t                VEC_CAPACITY = sizeof...(FieldTypes)*3;

    template<typename DecoderType>
    bool set(DecoderType& decoder)
    {
        // TODO - do we need to check if the fields are in sequence when validate is set to true ?

        while(setSubField(decoder))
        {
            if(!decoder.next())
            {
                break;
            }
        }

        // rewind the last decoded field
        if(decoder.isValid())
        {
            decoder.rewind();
        }

        // optional validation check
        if(!validate<VALIDATE>(decoder))
        {
            return false;
        }

        return true;
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

    template<typename T>
    bool setSubField(int fid, T& val)
    {
        return setSubFieldUnwind(fid, val, typename gens<sizeof...(FieldTypes)>::type());
    }

    bool getSubField(int fid, CachedField& retField)
    {
        return getSubFieldUnwind(fid, retField, typename gens<sizeof...(FieldTypes)>::type());
    }

    template<typename GroupType>
    bool getSubGroup(int fid, GroupType& retField)
    {
        return getSubGroupUnwind(fid, retField, typename gens<sizeof...(FieldTypes)>::type());
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
    template<bool T, typename DecoderType>
    typename std::enable_if<T, bool>::type validate(DecoderType& decoder)
    {
        // make sure all required fields are set and is in sequence
        return checkMessageSeq(decoder);
    }

    template<bool T, typename DecoderType>
    typename std::enable_if<!T, bool>::type validate(DecoderType& decoder)
    {
        return true;
    }

    template<typename DecoderType>
    bool checkMessageSeq(DecoderType& decoder)
    {
        decoder.reset();
        checkSeqUnwind(decoder, typename gens<sizeof...(FieldTypes)>::type());
    }

    // setIoVec
    template<int ...S>
    bool setIoVecUnwind(iovec*& vec, seq<S...>)
    {
        return setIoVec(vec, std::get<S>(_fieldList) ...);
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

    template<typename DecoderType, int ...S>
    bool checkSeqUnwind(DecoderType& decoder, seq<S...>)
    {
        return checkMessageSeq(decoder, std::get<S>(_fieldList) ...);
    }

    template<typename DecoderType, typename FieldType, typename... FieldTypeList>
    bool checkMessageSeq(DecoderType& decoder, FieldType& field, FieldTypeList&... fieldList)
    {
        if(field.IS_REQUIRED)
        {
            if(!decoder.isValid() || decoder.currentField().first != field.FID)
            {
                // mandatory not in sequence
                return false;
            }
            else // decoder.currentField().first == field.FID
            {
                decoder.next();
            }
        }
        else
        {
            if(decoder.isValid() && decoder.currentField().first == field.FID)
            {
                decoder.next();
            }
        }

        return checkMessageSeq(decoder, fieldList...);
    }

    template<typename DecoderType, typename FieldType>
    bool checkMessageSeq(DecoderType& decoder, FieldType& field)
    {
        if(field.IS_REQUIRED)
        {
            if(!decoder.isValid() || decoder.currentField().first != field.FID)
            {
                // mandatory not in sequence
                return false;
            }
            else // decoder.currentField().first == field.FID
            {
                decoder.next();
            }
        }
        else
        {
            if(!decoder.isValid() || (decoder.currentField().first == field.FID && decoder.isLast()))
            {
                return true;
            }
        }

        return false;
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

    template<typename T, int ...S>
    bool setSubFieldUnwind(int fid, T& val, seq<S...>)
    {
        return setSubField(fid, val, std::get<S>(_fieldList) ...);
    }

    template<typename T, typename FieldType, typename... FieldTypeList>
    bool setSubField(int fid, T& val, FieldType& field, FieldTypeList&... fieldList)
    {
        if(fid == field.FID)
        {
            // found it
            field.setValue(val);
            return true;
        }

        return setSubField(fid, val, fieldList...);
    }

    template<typename T, typename FieldType>
    bool setSubField(int fid, T& val, FieldType& field)
    {
        if(fid == field.FID)
        {
            // found it
            field.setValue(val);
            return true;
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

    // getSubField
    template<int ...S>
    bool getSubFieldUnwind(int fid, CachedField& retField, seq<S...>)
    {
        return getSubField(fid, retField, std::get<S>(_fieldList) ...);
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

    std::tuple<FieldTypes...>   _fieldList;

};

template<typename Validate, typename... FieldTypes>
constexpr int SFIXCollection<Validate, FieldTypes...>::FID;

template<typename Validate, typename... FieldTypes>
constexpr StringConstant SFIXCollection<Validate, FieldTypes...>::NAME;

template<typename Validate, typename... FieldTypes>
constexpr bool SFIXCollection<Validate, FieldTypes...>::IS_GROUP;

template<typename Validate, typename... FieldTypes>
constexpr bool SFIXCollection<Validate, FieldTypes...>::IS_REQUIRED;

template<typename Validate, typename... FieldTypes>
constexpr bool SFIXCollection<Validate, FieldTypes...>::VALIDATE;

template<typename Validate, typename... FieldTypes>
constexpr size_t SFIXCollection<Validate, FieldTypes...>::VEC_CAPACITY;

} // namespace vf_fix



#endif /* SFIXCOLLECTIONS_H_ */
