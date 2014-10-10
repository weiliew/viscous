/*
 * StringConstant.h
 *
 *  Created on: 16 Jun 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

// This class is adapted from Scott Schurr's str_const presented at C++ Now 2012
// https://github.com/boostcon/cppnow_presentations_2012/blob/master/wed/schurr_cpp11_tools_for_class_authors.pdf?raw=true
// http://2012.cppnow.org/

#ifndef STRINGCONSTANT_H_
#define STRINGCONSTANT_H_

namespace vf_common
{

class StringConstant
{
public:
    template<std::size_t Size>
    constexpr StringConstant(const char(&a)[Size])
    : _data(a)
    , _size(Size-1)
    {
        static_assert(Size >= 1, "Invalid string size");
    }

    constexpr char operator[](std::size_t n)
    {
        return n < _size ? _data[n] : throw std::out_of_range("");
    }

    constexpr std::size_t size()
    {
        return _size;
    }

    constexpr operator const char *()
    {
        return _data;
    }

    constexpr const char * data()
    {
        return _data;
    }

private:
    const char* const _data;
    const std::size_t _size;
};

class StringConstantArr
{
public:
    template<std::size_t Size>
    constexpr StringConstantArr(const StringConstant(&val)[Size])
    : _data(val)
    , _size(Size)
    {
    }

    constexpr StringConstantArr()
    : _data(0)
    , _size(0)
    {
    }

    constexpr const StringConstant& operator[](std::size_t n)
    {
        return (n < _size) ? _data[n] : throw std::out_of_range("");
    }

    constexpr std::size_t size()
    {
        return _size;
    }

    constexpr int getIndex(const StringConstant& rhs, size_t index = 0)
    {
        return rhs == _data[index] ? index : // found
                   index >= _size ? -1 :     // not found
                       getIndex(rhs, ++index);
    }

    int getIndex(const char * rhs, size_t index = 0) const
    {
        // TODO - non constexpr - how to make this fast ???
        for(size_t index=0;index<_size;++index)
        {
            if(!strcmp(rhs, _data[index]))
            {
                return index;
            }
        }

        return -1;
    }

private:
    const StringConstant*  _data;
    const std::size_t      _size;
};

}  // namespace vf_common



#endif /* STRINGCONSTANT_H_ */
