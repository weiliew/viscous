/*
 * FileSink.h
 *
 *  Created on: 11 Jan 2014
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FILESINK_H_
#define FILESINK_H_

#include <iostream>
#include <time.h>

namespace vf_common
{

class FileSink
{
public:
    FileSink(const char * fileName)
    : _fileName(fileName)
    {
    }

    virtual ~FileSink(){}

    void open()
    {
        if(!_ofstream.is_open())
        {
            _ofstream.open(_fileName.c_str(), std::ios_base::app);
            char timeStr[48];
            struct timeval currtimeval;
            gettimeofday( &currtimeval, NULL );
            time_t curtime =currtimeval.tv_sec;

            struct tm local;
            localtime_r(&curtime, &local);
            strftime(timeStr,48,"%Y/%m/%d %H:%M:%S", &local);
            _ofstream << "Log file opened: " << timeStr << std::endl;
        }
    }

    void close()
    {
        if(_ofstream.is_open())
        {
            char timeStr[48];
            struct timeval currtimeval;
            gettimeofday( &currtimeval, NULL );
            time_t curtime =currtimeval.tv_sec;

            struct tm local;
            localtime_r(&curtime, &local);
            strftime(timeStr,48,"%Y/%m/%d %H:%M:%S", &local);
            _ofstream << "Log file closed: " << timeStr << std::endl;
            _ofstream.flush();
            _ofstream.close();
        }
    }

    std::ostream& operator<< (const char val[])
    {
        return (_ofstream << val);
    }

    std::ostream& operator<< (std::ostream& val)
    {
        return (_ofstream << val);
    }

private:
    std::string   _fileName;
    std::ofstream _ofstream;
};

// TODO - threaded file log sink


}  // namespace vf_common


#endif /* FILESINK_H_ */
