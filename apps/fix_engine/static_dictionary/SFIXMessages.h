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

template<const char * MsgTypeStr>
class SFIXMessage
{
public:
    SFIXMessage()
    {
    }

    ~SFIXMessage()
    {
    }


    constexpr const char *          TYPE        = MsgTypeStr;
    constexpr const char *          NAME        = "";


private:

};

}  // namespace vf_fix



#endif /* SFIXMESSAGES_H_ */
