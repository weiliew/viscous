/*
 * StdoutSink.h
 *
 *  Created on: 11 Jan 2014
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */


#ifndef STDOUTSINK_H_
#define STDOUTSINK_H_

#include <iostream>
#include <time.h>

namespace vf_common
{

class StdoutSink
{
public:
    StdoutSink(){}
    virtual ~StdoutSink(){}

    void open() {}
    void close() {}

    std::ostream& operator<< (const char val[])
    {
        return (std::cout << val);
    }

    std::ostream& operator<< (std::ostream& val)
    {
        return (std::cout << val);
    }

private:
};

}  // namespace vf_common


#endif /* STDOUTSINK_H_ */
