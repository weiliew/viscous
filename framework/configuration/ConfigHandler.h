/*
 * ConfigHandler.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef CONFIGHANDLER_H_
#define CONFIGHANDLER_H_

#include <cstring>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace vf_common
{

class ConfigHandler
{
public:
    ConfigHandler()
    : _initialised (false)
    {}

    virtual ~ConfigHandler(){}

    bool loadconfig(int argc, char **argv)
    {
        boost::mutex::scoped_lock locl(_mutex);
        _configVarMap.clear();

        _configOptions.add_options()
            ("help",                                                                                  "# help message")
            ("ConfigFileType",  boost::program_options::value<std::string>()->default_value("xml"),   "# configuration file type (one of xml, json, ini and info")
            ("ConfigFile",      boost::program_options::value<std::string>(),                         "# configuration file name");

        if(argc && argv)
        {
            boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(_configOptions).allow_unregistered().run(), _configVarMap);
        }

        boost::program_options::notify(_configVarMap);

        if(!_configVarMap.count("ConfigFile") || !_configVarMap.count("ConfigFileType"))
        {
            _configOptions.print(std::cout);
            return false;
        }

        std::string fileType = _configVarMap["ConfigFileType"].as<std::string>();
        std::string fileName = _configVarMap["ConfigFile"].as<std::string>();

        if(!fileType.compare("xml"))
        {
            boost::property_tree::read_xml(fileName, _rootCfgTree);
        }
        else if(!fileType.compare("json"))
        {
            boost::property_tree::read_json(fileName, _rootCfgTree);
        }
        else if(!fileType.compare("ini"))
        {
            boost::property_tree::read_ini(fileName, _rootCfgTree);
        }
        else if(!fileType.compare("info"))
        {
            boost::property_tree::read_info(fileName, _rootCfgTree);
        }
        else
        {
            _configOptions.print(std::cout);
            return false;
        }

        _initialised = true;
        return true;
    }

    bool getNode(const char * name, boost::property_tree::ptree& node)
    {
        if(!name || !_initialised)
        {
            return false;
        }

        try
        {
            node =  _rootCfgTree.get_child(name);
        }
        catch(...)
        {
            return false;
        }

        return true;
    }

    template<typename ConfigType>
    bool getValue(const char * name, ConfigType& value)
    {
        // TODO - add config to known list to handle onChange callback

        if(!name || !_initialised)
        {
            return false;
        }

        try
        {
            value =  _rootCfgTree.get<ConfigType>(name);
        }
        catch(...)
        {
            return false;
        }
    }

private:
    boost::mutex                                    _mutex;
    boost::program_options::options_description     _configOptions;
    boost::program_options::variables_map           _configVarMap;

    boost::property_tree::ptree                     _rootCfgTree;
    bool                                            _initialised;

};

}  // namespace vf_common


#endif /* CONFIGHANDLER_H_ */
