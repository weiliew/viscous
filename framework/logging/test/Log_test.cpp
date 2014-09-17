/*
 * Log_test.cpp
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei.liew@outlook.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define BOOST_TEST_MODULE Log_test
#include <boost/test/included/unit_test.hpp>
#include <boost/thread.hpp>
#include "logging/Log.h"
#include "logging/FileSink.h"
#include "logging/StdoutSink.h"
#include <sys/syscall.h>
#include <stdio.h>

using namespace vf_common;

template<typename LogType>
bool checkLogger(LogType& logger)
{
    switch(logger.getLevel())
    {
    case LogDebug:
        BOOST_TEST_MESSAGE("Running logger test logger level DEBUG");
        break;
    case LogInfo:
        BOOST_TEST_MESSAGE("Running logger test logger level INFO");
        break;
    case LogWarn:
        BOOST_TEST_MESSAGE("Running logger test logger level WARN");
        break;
    case LogError:
        BOOST_TEST_MESSAGE("Running logger test logger level ERROR");
        break;
    case LogFatal:
        BOOST_TEST_MESSAGE("Running logger test logger level FATAL");
        break;
    default:
        BOOST_CHECK(false);
        break;
    }

    VF_LOG_DEBUG(logger, " Test logger debug for type: " << logger.getLevelStr());
    VF_LOG_INFO(logger,  " Test logger info  for type: " << logger.getLevelStr());
    VF_LOG_WARN(logger, " Test logger warn  for type: " << logger.getLevelStr());
    VF_LOG_ERROR(logger, " Test logger error for type: " << logger.getLevelStr());
    VF_LOG_FATAL(logger, " Test logger fatal for type: " << logger.getLevelStr());

    return true;
}

template<typename LogSink>
void testLogSink(LogSink& sink)
{
    Logger<LogSink> infoLogger(sink);
    Logger<LogSink> debugLogger(sink, LogDebug);
    Logger<LogSink> warnLogger(sink, LogWarn);
    Logger<LogSink> errorLogger(sink, LogError);
    Logger<LogSink> fatalLogger(sink, LogFatal);

    BOOST_CHECK(checkLogger(infoLogger));
    BOOST_CHECK(checkLogger(debugLogger));
    BOOST_CHECK(checkLogger(warnLogger));
    BOOST_CHECK(checkLogger(errorLogger));
    BOOST_CHECK(checkLogger(fatalLogger));
}

BOOST_AUTO_TEST_CASE( Log_Test_1 )
{
    BOOST_TEST_MESSAGE("Running test case Log_Test_1");

    StdoutSink stdoutSink;
    stdoutSink.open();
    testLogSink(stdoutSink);
    stdoutSink.close();

    FileSink fileSink("Log_test.out");
    fileSink.open();
    testLogSink(fileSink);
    fileSink.close();

    BOOST_TEST_MESSAGE("Test case Log_Test_1 completed");
}

BOOST_AUTO_TEST_CASE( Log_Test_2 )
{
    BOOST_TEST_MESSAGE("Running test case Log_Test_2");

    {
        StdoutSink stdoutSink;
        stdoutSink.open();

        std::shared_ptr<boost::thread> logThread1 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        std::shared_ptr<boost::thread> logThread2 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        std::shared_ptr<boost::thread> logThread3 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        std::shared_ptr<boost::thread> logThread4 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        std::shared_ptr<boost::thread> logThread5 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        std::shared_ptr<boost::thread> logThread6 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        std::shared_ptr<boost::thread> logThread7 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        std::shared_ptr<boost::thread> logThread8 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        std::shared_ptr<boost::thread> logThread9 = std::make_shared<boost::thread>(std::bind(&testLogSink<StdoutSink>, boost::ref(stdoutSink)));
        sleep(5);
        stdoutSink.close();
    }

    {
        FileSink fileSink("Log_test.out");
        fileSink.open();
        std::shared_ptr<boost::thread> logThread1 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));
        std::shared_ptr<boost::thread> logThread2 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));
        std::shared_ptr<boost::thread> logThread3 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));
        std::shared_ptr<boost::thread> logThread4 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));
        std::shared_ptr<boost::thread> logThread5 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));
        std::shared_ptr<boost::thread> logThread6 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));
        std::shared_ptr<boost::thread> logThread7 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));
        std::shared_ptr<boost::thread> logThread8 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));
        std::shared_ptr<boost::thread> logThread9 = std::make_shared<boost::thread>(std::bind(&testLogSink<FileSink>, boost::ref(fileSink)));

        sleep(5);
        fileSink.close();
    }

    BOOST_TEST_MESSAGE("Test case Log_Test_2 completed");
}

BOOST_AUTO_TEST_CASE( Log_Test_3 )
{
    BOOST_TEST_MESSAGE("Running test case Log_Test_3");

    remove("Log_test.perf.out");

    FileSink fileSink("Log_test.perf.out");
    fileSink.open();

    Logger<FileSink> debugLogger(fileSink, LogDebug);
    Logger<FileSink> infoLogger(fileSink, LogInfo);

    struct timeval currTime;
    struct timeval endTime;
    gettimeofday(&currTime, NULL);
    int NumLogCount = 1000000;
    for(int count=0;count<NumLogCount;++count)
    {
        VF_LOG_DEBUG(debugLogger, " Test performance " << count);
    }
    gettimeofday(&endTime, NULL);

    uint64_t timediff = 1000000*(endTime.tv_sec - currTime.tv_sec) + (endTime.tv_usec - currTime.tv_usec);
    BOOST_TEST_MESSAGE("Time taken - logging on: " << timediff << " microseconds");
    BOOST_TEST_MESSAGE("Time taken - logging on (per message): " << (double) ((double)timediff/(double)NumLogCount) << " microseconds");

    sleep(1);
    gettimeofday(&currTime, NULL);
    for(int count=0;count<NumLogCount;++count)
    {
        VF_LOG_DEBUG(infoLogger, " Test performance " << count);
    }
    gettimeofday(&endTime, NULL);

    timediff = 1000000*(endTime.tv_sec - currTime.tv_sec) + (endTime.tv_usec - currTime.tv_usec);
    BOOST_TEST_MESSAGE("Time taken - logging off: " << timediff << " microseconds");
    BOOST_TEST_MESSAGE("Time taken - logging off (per message): " << (double) ((double)timediff/(double)NumLogCount) << " microseconds");

    fileSink.close();

    remove("Log_test.perf.out");

    BOOST_TEST_MESSAGE("Test case Log_Test_3 completed");
}





