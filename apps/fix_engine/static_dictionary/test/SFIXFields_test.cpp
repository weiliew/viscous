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
#include "apps/fix_engine/static_dictionary/FieldDefs.h"

using namespace vf_common;
using namespace vf_fix;

bool runTest(const std::string& input)
{
    FIXMessageDecoder<100> fieldsDecoder;
    if(!fieldsDecoder.parseBuffer((char *) input.c_str(), input.length()))
    {
        return false;
    }

    SFIXGroup_753<std::true_type, std::false_type, 5> testGroup1;
    if(!testGroup1.set(fieldsDecoder))
    {
        return false;
    }

    std::ostringstream ostr;
    testGroup1.toString(ostr);
    BOOST_MESSAGE(ostr.str());

    fieldsDecoder.reset();
    SFIXGroup_753<std::true_type, std::false_type, 5> testGroup2;
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
    BOOST_CHECK(runTest(std::string("753=1") + (char) SOH + "707=CASH" + (char) SOH + "708=0" + (char) SOH + "1055=GBP"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("753=1") + (char) SOH + "708=0" + (char) SOH + "1055=GBP"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("753=1") + (char) SOH + "1055=GBP"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("753=0")+ (char) SOH));
    BOOST_CHECK(!runTest(std::string("753=1") + (char) SOH + "705=TEST" + (char) SOH + "708=0" + (char) SOH + "1055=GBP"+ (char) SOH));
    BOOST_CHECK(!runTest(std::string("753=1") + (char) SOH + "707=XXX" + (char) SOH + "708=0" + (char) SOH + "1055=GBP"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("753=1") + (char) SOH + "707=CASH" + (char) SOH + "708=0" + (char) SOH + "1055=GBP" +
                                               (char) SOH + "707=CASH" + (char) SOH + "708=1" + (char) SOH + "1055=USD"+ (char) SOH));
    BOOST_CHECK(runTest(std::string("753=2") + (char) SOH + "707=CASH" + (char) SOH + "708=0" +
                                               (char) SOH + "707=CASH" + (char) SOH + "708=1" + (char) SOH));
    BOOST_CHECK(runTest(std::string("753=2") + (char) SOH + "708=0" + (char) SOH + "1055=GBP" +
                                               (char) SOH + "708=1" + (char) SOH + "1055=USD"+ (char) SOH));
    BOOST_CHECK(!runTest(std::string("753=2") + (char) SOH + "707=CASH" + (char) SOH + "708=0" + (char) SOH + "1055=GBP" +
                                               (char) SOH + "705=OTHER"+ (char) SOH));
    BOOST_CHECK(!runTest(std::string("753=3") + (char) SOH + "707=CASH" + (char) SOH + "708=0" + (char) SOH + "1055=GBP" +
                                               (char) SOH + "707=CASH" + (char) SOH + "708=1" + (char) SOH + "1055=USD"+ (char) SOH));
}


#endif /* SFIXFIELDS_TEST_CPP_ */
