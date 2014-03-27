/*
 * HashUtils.h
 *
 *  Created on: 13 Feb 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef HASHUTILS_H_
#define HASHUTILS_H_

#include <boost/functional/hash.hpp>
#include <string.h>

namespace vf_common
{

struct str_hash
{
    std::size_t operator()(const char* s) const
    {
        return boost::hash_range(s, s + strlen(s));
    }
};

struct str_lt
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        return strcmp(lhs, rhs) < 0;
    }
};

struct str_eq
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        return (strcmp(lhs, rhs) == 0);
    }
};

}


#endif /* HASHUTILS_H_ */
