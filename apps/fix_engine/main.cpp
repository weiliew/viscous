/*
 * main.cpp
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <iostream>

#include "logging/Log.h"
#include "configuration/ConfigHandler.h"
#include "configuration/Config.h"
#include "configuration/ConfigNode.h"
#include "signals/Signal.h"

#include "message/FIXMessagePoolDefs.h"

using namespace vf_common;
using namespace vf_fix;

// TODO - global statics for now
static ConfigHandler s_configHandler;

int main(int argc, char* argv[])
{
    /*
    try
    {
        // set up configurations and logging
        s_configHandler.loadconfig(argc, argv);

        StdoutSink stdoutSink;
        stdoutSink.open();
        Logger myLogger(&stdoutSink);


        Config<int> intCfg1(s_configHandler, "intcfg1", 0);
        Config<int> intCfg2(s_configHandler, "intcfg2", 0);


        boost::asio::io_service io;
        boost::asio::io_service::work myWork(io);
        boost::shared_ptr<boost::thread> thread1(new boost::thread(boost::bind(&boost::asio::io_service::run, &io)));
        boost::shared_ptr<boost::thread> thread2(new boost::thread(boost::bind(&boost::asio::io_service::run, &io)));
        boost::shared_ptr<boost::thread> thread3(new boost::thread(boost::bind(&boost::asio::io_service::run, &io)));
        boost::shared_ptr<boost::thread> thread4(new boost::thread(boost::bind(&boost::asio::io_service::run, &io)));
        boost::shared_ptr<boost::thread> thread5(new boost::thread(boost::bind(&boost::asio::io_service::run, &io)));

        SignalFactory<Signal<FIXMessage> > acceptorSignalFactory("AcceptSignal", io);
        TcpAcceptorHandler<BasicMessageFactory<FIXMessage>, SignalFactory<Signal<FIXMessage> >, FIXAcceptor<FIXSessionHandler> > acceptorHandler(io, myLogger, &acceptorSignalFactory);
        auto acceptor1 = acceptorHandler.startNewAcceptor("127.0.0.1", "55555");

        while(1)
        {
            sleep(1);
        }
    }
    catch(std::exception const & e)
    {
        return 1;
    }
    catch(...)
    {
        return 1;
    }
    */

    return 0;
}



