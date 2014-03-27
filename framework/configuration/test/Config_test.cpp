/*
 * Config_test.cpp
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE Config_test
#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include "configuration/ConfigHandler.h"
#include "configuration/Config.h"
#include "configuration/ConfigNode.h"
#include <sys/syscall.h>
#include <stdio.h>

using namespace vf_common;

// TODO - test onChange !!

template<typename ConfigType>
void printConfig(ConfigType& cfg)
{
    BOOST_TEST_MESSAGE("Configuration value for [" << cfg.getName() << "] is [" << cfg.getValue() << "]");
}

BOOST_AUTO_TEST_CASE( Config_Test_1 )
{
    BOOST_TEST_MESSAGE("Running test case Config_Test_1");

    ConfigHandler handler;

    handler.loadconfig(boost::unit_test::framework::master_test_suite().argc, boost::unit_test::framework::master_test_suite().argv);

    Config<int> intCfg1(handler, "intcfg1", 0);
    Config<int> intCfg2(handler, "intcfg2", 0);
    printConfig(intCfg1);
    printConfig(intCfg2);

    ConfigNode node1(handler, "group1");
    std::string value;
    node1.getConfig("strcfg1", value);
    BOOST_TEST_MESSAGE("Configuration value for [" << node1.getName() << "] is [" << value.c_str() << "]");

    node1.printNode(std::cout);

    BOOST_TEST_MESSAGE("Test case Config_Test_1 completed");
}


