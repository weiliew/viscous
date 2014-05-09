/*
 * MessagePool_test.cpp
 *
 *  Created on: 27 Apr 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE MessagePool_test
#include <sys/syscall.h>
#include <stdio.h>
#include <chrono>
#include <thread>

#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/asio/io_service.hpp>

#include "apps/fix_engine/message/FIXMessagePoolDefs.h"
#include "apps/fix_engine/message/FIXMessageDecoder.h"

using namespace vf_common;
using namespace vf_fix;

#define SOH 0x01
#define MSGSEP

template<typename FIXMsgType>
void testFIXMessage(FIXMsgType msg)
{
    BOOST_CHECK(msg->parseFIXMessage());
    BOOST_CHECK(msg->parsed() && msg->complete());
}

template<typename PoolDefs>
void runTest()
{
    // create and prime message factory
    typename PoolDefs::PooledFactoryType bufferFactory(0);

    // total of 10 FIX messages
    std::vector<std::string> inputArray = {
        std::string("8=FIX4.4") + (char) SOH + "9=",
        std::string("19") + (char) SOH + "35=D" + (char) SOH + "49=001" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH,
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=002" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH,
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=003" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=004" + (char) SOH,
        std::string("56=YYY") + (char) SOH + "10=123" + (char) SOH,
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=005" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=006" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=007" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH,
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=008" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=009" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH +
        std::string("8=FIX4.4") + (char) SOH + "9=19" + (char) SOH + "35=D" + (char) SOH + "49=010" + (char) SOH + "56=YYY" + (char) SOH + "10=123" + (char) SOH
    };

    for(int count=0;count<inputArray.size();++count)
    {
        auto msg = bufferFactory.create();
        msg->setBuffer(inputArray[count].data(), inputArray[count].length());
        if(msg->getFIXMsg())
        {
            testFIXMessage(msg);
        }

        while(bufferFactory.getCachedSize() > 20)
        {
            // attempt to consume the next FIX message from the cache
            auto cached = bufferFactory.create();
            if(cached->getFIXMsg())
            {
                testFIXMessage(cached);
            }
            else
            {
                // cached buffer is not a complete fix message
                break;
            }
        }
    }

    BOOST_MESSAGE("Test finished with factory type: " << typeid(bufferFactory).name());
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE( MessagePool_test_1 )
{
    runTest<LockingFIXMsg2k>();
}

BOOST_AUTO_TEST_CASE( MessagePool_test_2 )
{
    runTest<LockFreeFIXMsg2k>();
}




