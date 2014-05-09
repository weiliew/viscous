/*
 * FIXParser.h
 *
 *  Created on: 25 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXPARSER_H_
#define FIXPARSER_H_

namespace vf_apps
{

// Takes in a FIXMsg and arse each individual FIX fields and put the fields into a map/array

template<typename BufferType>
class FIXParser
{
public:
    FIXParser()
    {
    }

    ~FIXParser()
    {
    }

    void parseMessage(BufferType& buffer)
    {

    }

private:

};

}

#endif /* FIXPARSER_H_ */
