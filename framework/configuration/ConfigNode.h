/*
 * ConfigNode.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef CONFIGNODE_H_
#define CONFIGNODE_H_

#include "Config.h"

namespace vf_common
{

class ConfigNode : public ConfigBase
{
public:
    ConfigNode(ConfigHandler& handler, const char * name)
    : ConfigBase (handler, name)
    {
        _initialised = handler.getNode(name, _cfgNode);
    }

    ConfigNode(const ConfigNode& cpy)
    : ConfigBase (cpy)
    , _cfgNode (cpy._cfgNode)
    {

    }

    virtual ~ConfigNode()
    {

    }

    ConfigNode getChildNode(const char * name)
    {
        return ConfigNode(_handler, name);
    }

    template<typename ValueType>
    bool getConfig(const char * name, ValueType& value)
    {
        if(!_initialised || !name)
        {
            return false;
        }

        try
        {
            value =  _cfgNode.get<ValueType>(name);
        }
        catch(...)
        {
            return false;
        }
    }

    void printNode(std::ostream& stream, const boost::property_tree::ptree * node = NULL)
    {
        if(!node)
        {
            node = &_cfgNode;
        }
        static int depth = 0;
        depth += 2;

        bool printValue = true;

        boost::property_tree::ptree::const_iterator iter = node->begin();
        while(iter != node->end())
        {
            printValue = false;
            stream << std::endl << std::setw(depth) << " " << (*node->begin()).first.c_str();
            printNode(stream, &((*iter).second));
            ++iter;
        }

        if(printValue)
        {
            stream << " = " << node->data().c_str() << std::endl;
        }
    }

private:
    boost::property_tree::ptree     _cfgNode;
};

}  // namespace vf_common


#endif /* CONFIGNODE_H_ */
