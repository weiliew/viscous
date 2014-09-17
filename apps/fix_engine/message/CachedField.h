/*
 * CachedField.h
 *
 *  Created on: 31 May 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef CACHEDFIELD_H_
#define CACHEDFIELD_H_

#include <string>

namespace vf_fix
{

template<typename T>
struct ValueType
{
    ValueType();

    T getVal(const char * val);

    T       _value;
    bool    _cached;
};

template<>
ValueType<uint64_t>::ValueType()
: _value(0)
, _cached(false)
{
}

template<>
ValueType<int64_t>::ValueType()
: _value(0)
, _cached(false)
{
}

template<>
ValueType<double>::ValueType()
: _value(0.0)
, _cached(false)
{
}

template<>
ValueType<bool>::ValueType()
: _value(false)
, _cached(false)
{
}

template<>
ValueType<std::string>::ValueType()
: _value()
, _cached(false)
{
}

template<>
uint64_t ValueType<uint64_t>::getVal(const char * val)
{
    if(_cached)
    {
        return _value;
    }

    _value = std::stoul(val, NULL, 0);
    _cached = true;
    return _value;
}

template<>
int64_t ValueType<int64_t>::getVal(const char * val)
{
    if(_cached)
    {
        return _value;
    }

    _value = std::stol(val, NULL, 0);
    _cached = true;
    return _value;
}

template<>
double ValueType<double>::getVal(const char * val)
{
    if(_cached)
    {
        return _value;
    }

    _value = std::stod(val, NULL);
    _cached = true;
    return _value;
}

template<>
bool ValueType<bool>::getVal(const char * val)
{
    if(_cached)
    {
        return _value;
    }

    _value = (*val == 'Y' || *val == 'y');
    _cached = true;
    return _value;
}

template<>
std::string ValueType<std::string>::getVal(const char * val)
{
    if(_cached)
    {
        return _value;
    }

    _value = val;
    _cached = true;
    return _value;
}

class CachedField
{
public:
    CachedField()
    : _val(NULL)
    {
    }

    template<typename T>
    T getValue();

    void setVal(const char * value)
    {
        _val = value;
    }

    const char * value()
    {
        return _val;
    }

    bool empty()
    {
        return (_val == NULL);
    }

    template<typename T, typename ToString = std::true_type>
    void setCachedVal(T& val);

    void clear()
    {
        _val = NULL;
        _unsignedVal._cached = false;
        _signedVal._cached = false;
        _doubleVal._cached = false;
        _boolVal._cached = false;
        _stringVal._cached = false;
    }

private:
    template<typename T>
    void applyToString(T& val)
    {
        // TODO - may want to use more efficient method here for T to string conversion
        _stringVal._value = std::to_string(val);
        _val = _stringVal._value.c_str();
    }

    const char *            _val;

    ValueType<uint64_t>     _unsignedVal;
    ValueType<int64_t>      _signedVal;
    ValueType<double>       _doubleVal;
    ValueType<bool>         _boolVal;
    ValueType<std::string>  _stringVal;
};

template<>
void CachedField::setCachedVal<uint64_t, std::true_type>(uint64_t& val)
{
    _unsignedVal._value = val;
    _unsignedVal._cached = true;
    applyToString(val);
}

template<>
void CachedField::setCachedVal<uint64_t, std::false_type>(uint64_t& val)
{
    _unsignedVal._value = val;
    _unsignedVal._cached = true;
}

template<>
void CachedField::setCachedVal<int64_t, std::true_type>(int64_t& val)
{
    _signedVal._value = val;
    _signedVal._cached = true;
    applyToString(val);
}

template<>
void CachedField::setCachedVal<int64_t, std::false_type>(int64_t& val)
{
    _signedVal._value = val;
    _signedVal._cached = true;
}

template<>
void CachedField::setCachedVal<uint32_t, std::true_type>(uint32_t& val)
{
    _unsignedVal._value = static_cast<uint64_t>(val);
    _unsignedVal._cached = true;
    applyToString(val);
}

template<>
void CachedField::setCachedVal<uint32_t, std::false_type>(uint32_t& val)
{
    _unsignedVal._value = static_cast<uint64_t>(val);
    _unsignedVal._cached = true;
}

template<>
void CachedField::setCachedVal<int32_t, std::true_type>(int32_t& val)
{
    _signedVal._value = static_cast<int64_t>(val);
    _signedVal._cached = true;
    applyToString(val);
}

template<>
void CachedField::setCachedVal<int32_t, std::false_type>(int32_t& val)
{
    _signedVal._value = static_cast<int64_t>(val);
    _signedVal._cached = true;
}

template<>
void CachedField::setCachedVal<double, std::true_type>(double& val)
{
    _doubleVal._value = val;
    _doubleVal._cached = true;
    applyToString(val);
}

template<>
void CachedField::setCachedVal<double, std::false_type>(double& val)
{
    _doubleVal._value = val;
    _doubleVal._cached = true;
}

template<>
void CachedField::setCachedVal<std::string, std::true_type>(std::string& val)
{
    _stringVal._value = val;
    _val = _stringVal._value.c_str();
    _stringVal._cached = true;
}

template<>
void CachedField::setCachedVal<std::string, std::false_type>(std::string& val)
{
    _stringVal._value = val;
}

template<>
void CachedField::setCachedVal<bool, std::true_type>(bool& val)
{
    _boolVal._value = val;
    _boolVal._cached = true;

    if(val)
    {
        _stringVal._value = "Y";
    }
    else
    {
        _stringVal._value = "N";
    }

    _val = _stringVal._value.c_str();
    _stringVal._cached = true;
}

template<>
void CachedField::setCachedVal<bool, std::false_type>(bool& val)
{
    _boolVal._value = val;
    _boolVal._cached = true;}

template<>
uint64_t CachedField::getValue<uint64_t>()
{
    // cached ?
    if(_unsignedVal._cached)
    {
        return _unsignedVal._value;
    }

    if(UNLIKELY(!_val))
    {
        return 0;
    }

    return _unsignedVal.getVal(_val);
}

template<>
int64_t CachedField::getValue<int64_t>()
{
    // cached ?
    if(_signedVal._cached)
    {
        return _signedVal._value;
    }


    if(UNLIKELY(!_val))
    {
        return 0;
    }

    return _signedVal.getVal(_val);
}

template<>
uint32_t CachedField::getValue<uint32_t>()
{
    // cached ?
    if(_unsignedVal._cached)
    {
        return _unsignedVal._value;
    }

    if(UNLIKELY(!_val))
    {
        return 0;
    }

    return static_cast<uint32_t>(_unsignedVal.getVal(_val));
}

template<>
int32_t CachedField::getValue<int32_t>()
{
    // cached ?
    if(_signedVal._cached)
    {
        return _signedVal._value;
    }

    if(UNLIKELY(!_val))
    {
        return 0;
    }

    return static_cast<int32_t>(_signedVal.getVal(_val));
}

template<>
double CachedField::getValue<double>()
{
    // cached ?
    if(_doubleVal._cached)
    {
        return _doubleVal._value;
    }

    if(UNLIKELY(!_val))
    {
        return 0.0;
    }

    return _doubleVal.getVal(_val);
}

template<>
bool CachedField::getValue<bool>()
{    // cached ?
    if(_boolVal._cached)
    {
        return _boolVal._value;
    }

    if(UNLIKELY(!_val))
    {
        return false;
    }

    return _boolVal.getVal(_val);
}

template<>
std::string CachedField::getValue<std::string>()
{    // cached ?
    if(_stringVal._cached)
    {
        return _stringVal._value;
    }

    if(UNLIKELY(!_val))
    {
        return std::string();
    }

    return _stringVal.getVal(_val);
}

}  // namespace vf_fix



#endif /* CACHEDFIELD_H_ */
