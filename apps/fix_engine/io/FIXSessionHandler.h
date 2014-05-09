/*
 * FIXSessionHandler.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef FIXSESSIONHANDLER_H_
#define FIXSESSIONHANDLER_H_

#include "logging/Log.h"
#include "fix/message/FIXDictionary.h"

namespace osf_fix
{

class FIXSessionHandler
{
public:
    FIXSessionHandler(Logger& logger)
    : _logger(logger)
    , _dataDictionary(logger)
    // TODO - , _cfgDataDictionary(s_configHandler, "intcfg1", 0)
    {

    }

    virtual ~FIXSessionHandler()
    {

    }

    virtual bool parseFIXMessage(boost::shared_ptr<FIXMessage> message, bool validate = true)
    {
        if(!_dataDictionary.isParsed())
        {
            // TODO - use config
            _dataDictionary.loadDictionary("../../../source/fix/cfgfiles/FIX44.xml");
        }

        // TODO - temp vars - move into map
        unsigned char * fidList[1024];
        unsigned char * valList[1024];

        memset(fidList, '\0', 1024*sizeof(unsigned char*));
        memset(valList, '\0', 1024*sizeof(unsigned char*));

        // fully parse the fix message and checks validity if necessary
        unsigned char * buffer = message->getMessage();
        unsigned int    bufferLen = message->getMessageLen();
        unsigned int    numFields = 0;

        unsigned char * fidPtr = buffer;
        unsigned char * valPtr = NULL;
        for(unsigned int count = 0; count < bufferLen; ++count)
        {
            if(buffer[count] == SOH)
            {
                buffer[count] = NULL;

                // store the field if available
                if(fidPtr && valPtr)
                {
                    fidList[numFields] = fidPtr;
                    valList[numFields] = valPtr;
                    ++numFields;
                }

                // next is a fid - or this is the last field
                fidPtr = &buffer[count+1];
                valPtr = NULL;
            }
            else if(buffer[count] == '=')
            {
                // next is value
                buffer[count] = NULL;
                valPtr = &buffer[count+1];
            }
        }

        // Lets print the fix message out
        std::stringstream str;
        for(unsigned int count = 0; count<numFields;++count)
        {
            str << fidList[count] << "=" << valList[count] << "\n";
        }
        OSF_LOG_DEBUG(_logger, "Incoming FIX message:\n" << str.str());

        return true;
    }

    virtual bool onReceive()
    {
        // called as soon as a FIX message is received before any processing is done
        return true;
    }

    virtual bool onLogin()
    {
        // received 35=A
        return true;
    }

    virtual bool onAdmin()
    {
        // received admin message
        return true;
    }

    virtual bool onMessage()
    {
        // received a message
        return true;
    }

private:
    Logger _logger;

    // TODO - this class should be creating and storing the session info for each incoming connection etc -
    // each client will have their own data dictionary. For now, we use the same one ...
    FIXDictionary   _dataDictionary;
    // TODO - Config<int>     _cfgDataDictionary;
};

}  // namespace osf_fix


#endif /* FIXSESSIONHANDLER_H_ */
