/*
 * FIXDictionary_test.cpp
 *
 *  Created on: 18 May 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */


#define BOOST_TEST_MODULE FIXDictionary_test
#include <sys/syscall.h>
#include <stdio.h>
#include <chrono>
#include <thread>

#include <boost/test/included/unit_test.hpp>
#include "logging/Log.h"
#include "logging/StdoutSink.h"
#include "apps/fix_engine/dictionary/FIXDictionary.h"

using namespace vf_common;
using namespace vf_fix;

BOOST_AUTO_TEST_CASE( FIXDictionary_test_1 )
{
    StdoutSink stdoutSink;
    stdoutSink.open();
    Logger<StdoutSink> logger(stdoutSink, LogDebug);

    FIXDictionary<Logger<StdoutSink>> dict(logger);
    dict.load(std::string(boost::unit_test::framework::master_test_suite().argv[1]) + "/FIX40.xml");

    VF_LOG_INFO(logger, dict);
}

BOOST_AUTO_TEST_CASE( FIXDictionary_test_2 )
{
    StdoutSink stdoutSink;
    stdoutSink.open();
    Logger<StdoutSink> logger(stdoutSink, LogDebug);

    FIXDictionary<Logger<StdoutSink>> dict(logger);
    dict.load(std::string(boost::unit_test::framework::master_test_suite().argv[1]) + "/FIX41.xml");

    VF_LOG_INFO(logger, dict);
}

BOOST_AUTO_TEST_CASE( FIXDictionary_test_3 )
{
    StdoutSink stdoutSink;
    stdoutSink.open();
    Logger<StdoutSink> logger(stdoutSink, LogDebug);

    FIXDictionary<Logger<StdoutSink>> dict(logger);
    dict.load(std::string(boost::unit_test::framework::master_test_suite().argv[1]) + "/FIX42.xml");

    VF_LOG_INFO(logger, dict);
}

BOOST_AUTO_TEST_CASE( FIXDictionary_test_4 )
{
    StdoutSink stdoutSink;
    stdoutSink.open();
    Logger<StdoutSink> logger(stdoutSink, LogDebug);

    FIXDictionary<Logger<StdoutSink>> dict(logger);
    dict.load(std::string(boost::unit_test::framework::master_test_suite().argv[1]) + "/FIX43.xml");

    VF_LOG_INFO(logger, dict);
}

BOOST_AUTO_TEST_CASE( FIXDictionary_test_5 )
{
    StdoutSink stdoutSink;
    stdoutSink.open();
    Logger<StdoutSink> logger(stdoutSink, LogDebug);

    FIXDictionary<Logger<StdoutSink>> dict(logger);
    dict.load(std::string(boost::unit_test::framework::master_test_suite().argv[1]) + "/FIX44.xml");

    VF_LOG_INFO(logger, dict);
}
