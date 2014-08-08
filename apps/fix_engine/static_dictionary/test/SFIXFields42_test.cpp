/*
 * SFIXFields_test.cpp
 *
 *  Created on: 8 Jun 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef SFIXFIELDS_TEST_CPP_
#define SFIXFIELDS_TEST_CPP_

#define BOOST_TEST_MODULE SFIXFields_test
#include <sys/syscall.h>
#include <stdio.h>
#include <chrono>
#include <thread>

#include <boost/test/included/unit_test.hpp>

#include "apps/fix_engine/message/FIXMessageDecoder.h"
#include "apps/fix_engine/static_dictionary/gen/FieldDefsGen42.h"

using namespace vf_common;
using namespace vf_fix;

bool runTest(const std::string& input)
{
    FIXMessageDecoder<100> fieldsDecoder;
    if(!fieldsDecoder.parseBuffer((char *) input.c_str(), input.length()))
    {
        return false;
    }

    fix_defs::messages::Allocation::SFIXGroup_NoMiscFees<std::true_type, std::false_type, 5> testGroup1;
    if(!testGroup1.set(fieldsDecoder))
    {
        return false;
    }

    std::ostringstream ostr;
    testGroup1.toString(ostr);
    BOOST_MESSAGE(ostr.str());

    fieldsDecoder.reset();
    fix_defs::messages::Allocation::SFIXGroup_NoMiscFees<std::true_type, std::true_type, 5> testGroup2;
    if(!testGroup2.set(fieldsDecoder))
    {
        return false;
    }

    std::ostringstream ostr2;
    testGroup2.toString(ostr2);
    BOOST_MESSAGE(ostr2.str());

    return true;
}

BOOST_AUTO_TEST_CASE( MessagePool_test_1 )
{
    BOOST_CHECK(runTest(std::string("136=1") + (char) SOH + "137=10.00" + (char) SOH + "138=X" + (char) SOH + "139=1"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("136=1") + (char) SOH + "138=X" + (char) SOH + "139=1"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("136=1") + (char) SOH + "139=1"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("136=0")+ (char) SOH));
    BOOST_CHECK(!runTest(std::string("136=1") + (char) SOH + "705=TEST" + (char) SOH + "138=X" + (char) SOH + "139=1"+ (char) SOH));
    BOOST_CHECK(!runTest(std::string("136=1") + (char) SOH + "707=XXX" + (char) SOH + "138=X" + (char) SOH + "139=1"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("136=2") + (char) SOH + "137=10.00" + (char) SOH + "138=X" + (char) SOH + "139=1" +
                                               (char) SOH + "137=10.00" + (char) SOH + "138=Y" + (char) SOH + "139=2"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("136=2") + (char) SOH + "137=10.00" + (char) SOH + "138=X" +
                                               (char) SOH + "137=10.00" + (char) SOH + "138=Y" + (char) SOH));
    BOOST_CHECK(runTest(std::string("136=2") + (char) SOH + "138=X" + (char) SOH + "139=1" +
                                               (char) SOH + "138=Y" + (char) SOH + "139=2"+ (char) SOH));
    BOOST_CHECK(!runTest(std::string("136=2") + (char) SOH + "137=10.00" + (char) SOH + "138=X" + (char) SOH + "139=1" +
                                               (char) SOH + "705=OTHER"+ (char) SOH));
    BOOST_CHECK(!runTest(std::string("136=3") + (char) SOH + "137=10.00" + (char) SOH + "138=X" + (char) SOH + "139=1" +
                                               (char) SOH + "137=10.00" + (char) SOH + "138=A" + (char) SOH + "139=2"+ (char) SOH));

    std::string msgTestStr(std::string("8=FIX4.0") + (char) SOH +
                                       "9=100" + (char) SOH + // not real length - just for testing
                                       "35=M" + (char) SOH +
                                       "49=sender" + (char) SOH +
                                       "56=target" + (char) SOH +
                                       "34=1" + (char) SOH +
                                       "97=N" + (char) SOH +
                                       "52=31/7/2014 17:35:00.000" + (char) SOH +
                                       "66=100" + (char) SOH +
                                       "10=123" + (char) SOH);
    FIXMessageDecoder<100> fieldsDecoder;
    BOOST_CHECK(fieldsDecoder.parseBuffer((char *) msgTestStr.c_str(), msgTestStr.length()));

    fix_defs::messages::ListStatus::SFIXMessage_ListStatus<std::false_type, 10> msg1;
    BOOST_CHECK(msg1.set(fieldsDecoder));

    std::ostringstream ostr;
    msg1.toString(ostr);
    BOOST_MESSAGE(ostr.str());

    fieldsDecoder.reset();
    fix_defs::messages::ListStatus::SFIXMessage_ListStatus<std::true_type, 10> msg2;
    BOOST_CHECK(msg2.set(fieldsDecoder));

    std::ostringstream ostr2;
    msg2.toString(ostr2);
    BOOST_MESSAGE(ostr2.str());

    {
    // out of order hdr seq check
    std::string msgTestStr(std::string("8=FIX4.0") + (char) SOH +
                                           "35=M" + (char) SOH +
                                           "9=100" + (char) SOH + // not real length - just for testing
                                           "49=sender" + (char) SOH +
                                           "56=target" + (char) SOH +
                                           "34=1" + (char) SOH +
                                           "97=N" + (char) SOH +
                                           "52=31/7/2014 17:35:00.000" + (char) SOH +
                                           "66=100" + (char) SOH +
                                           "10=123" + (char) SOH);
    BOOST_CHECK(fieldsDecoder.parseBuffer((char *) msgTestStr.c_str(), msgTestStr.length()));
    BOOST_CHECK(msg1.set(fieldsDecoder));
    BOOST_CHECK(!msg2.set(fieldsDecoder));
    }

    {
    // missing mandatory msg check
    std::string msgTestStr = (std::string("8=FIX4.0") + (char) SOH +
                                        "9=100" + (char) SOH + // not real length - just for testing
                                        "35=M" + (char) SOH +
                                       "49=sender" + (char) SOH +
                                       "56=target" + (char) SOH +
                                       "34=1" + (char) SOH +
                                       "97=N" + (char) SOH +
                                       "52=31/7/2014 17:35:00.000" + (char) SOH +
                                       "66=100" + (char) SOH);
    BOOST_CHECK(fieldsDecoder.parseBuffer((char *) msgTestStr.c_str(), msgTestStr.length()));
    BOOST_CHECK(msg1.set(fieldsDecoder));
    BOOST_CHECK(!msg2.set(fieldsDecoder));
    }

    {
    // missing mandatory hdr seq check
    std::string msgTestStr(std::string("8=FIX4.0") + (char) SOH +
                                           "9=100" + (char) SOH + // not real length - just for testing
                                           "49=sender" + (char) SOH +
                                           "56=target" + (char) SOH +
                                           "34=1" + (char) SOH +
                                           "97=N" + (char) SOH +
                                           "52=31/7/2014 17:35:00.000" + (char) SOH +
                                           "66=100" + (char) SOH +
                                           "10=123" + (char) SOH);
    BOOST_CHECK(fieldsDecoder.parseBuffer((char *) msgTestStr.c_str(), msgTestStr.length()));
    BOOST_CHECK(msg1.set(fieldsDecoder));
    BOOST_CHECK(!msg2.set(fieldsDecoder));
    }
}


#endif /* SFIXFIELDS_TEST_CPP_ */
