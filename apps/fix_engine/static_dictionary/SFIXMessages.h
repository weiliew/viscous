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

namespace vf_fix
{

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

    bool getHeaderField(int fid, CachedField& retField)
    {
        return _header.getSubField(fid, retField);
    }

    bool getTrailerField(int fid, CachedField& retField)
    {
        return _trailer.getSubField(fid, retField);
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
};

template<const StringConstant& Name, const StringConstant& Type, typename Validate, typename HeaderType, typename TrailerType, typename... FieldTypes>
constexpr StringConstant SFIXMessage<Name, Type, Validate, HeaderType, TrailerType, FieldTypes...>::NAME;

template<const StringConstant& Name, const StringConstant& Type, typename Validate, typename HeaderType, typename TrailerType, typename... FieldTypes>
constexpr StringConstant SFIXMessage<Name, Type, Validate, HeaderType, TrailerType, FieldTypes...>::TYPE;

template<const StringConstant& Name, const StringConstant& Type, typename Validate, typename HeaderType, typename TrailerType, typename... FieldTypes>
constexpr bool SFIXMessage<Name, Type, Validate, HeaderType, TrailerType, FieldTypes...>::VALIDATE;

}  // namespace vf_fix



#endif /* SFIXMESSAGES_H_ */
