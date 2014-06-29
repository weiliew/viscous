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

namespace vf_fix
{


class CachedField
{
public:
    CachedField()
    : _val          (NULL)
    , _longVal      (0)
    , _longCached   (false)
    , _doubleVal    (0.0)
    , _doubleCached (false)
    , _boolVal      (false)
    , _boolCached   (false)
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

private:
    const char *    _val;
    long            _longVal;
    bool            _longCached;
    double          _doubleVal;
    bool            _doubleCached;
    bool            _boolVal;
    bool            _boolCached;
};

template<>
long CachedField::getValue<long>()
{
    if(UNLIKELY(!_val))
    {
        return 0;
    }

    if(!_longCached)
    {
        _longVal = atol(_val);
        _longCached = true;
    }

    return _longVal;
}

template<>
double CachedField::getValue<double>()
{
    if(UNLIKELY(!_val))
    {
        return 0.0;
    }

    if(!_doubleCached)
    {
        _doubleVal = atof(_val);
        _doubleCached = true;
    }

    return _doubleVal;
}

template<>
bool CachedField::getValue<bool>()
{
    if(UNLIKELY(!_val))
    {
        return false;
    }

    if(!_boolCached)
    {
        _boolVal = (*_val == 'Y' || *_val == 'y');
        _boolCached = true;
    }

    return _boolVal;
}


}  // namespace vf_fix



#endif /* CACHEDFIELD_H_ */
