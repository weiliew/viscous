/*
 * FIXMessageDecoder.h
 *
 *  Created on: 27 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIXMESSAGEDECODER_H_
#define FIXMESSAGEDECODER_H_

namespace vf_fix
{

// this class allows a single FIX message to be decoded and the FIX fields to be walked by the caller
// Its intention is that the caller will have a re-usable decoder to parse all the incoming messages
template<typename MsgType>
class FIXMessageDecoder
{
public:
    FIXMessageDecoder()
    {
    }

    virtual ~FIXMessageDecoder()
    {
    }

    bool decodeMessage(MsgType& msg)
    {
        char * buffer = msg.buffer();
        size_t len = msg.size();

        // scan and replace all SOH with NULL

        return true;
    }

private:

};

}  // namespace vf_fix



#endif /* FIXMESSAGEDECODER_H_ */
