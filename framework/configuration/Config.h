/*
 * Config.h
 *
 *  Created on: 1 Jan 2013
 *      Author: Wei Liew [wei@onesixeightsolutions.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <cstring>

namespace vf_common
{

class ConfigBase
{
public:
    ConfigBase(ConfigHandler& handler, const char * name)
    : _name         (name)
    , _initialised  (false)
    , _handler      (handler)
    {

    }

    ConfigBase(const ConfigBase& cpy)
    : _name         (cpy._name)
    , _initialised  (cpy._initialised)
    , _handler      (cpy._handler)
    {

    }

    const char * getName()
    {
        return _name.c_str();
    }

    void onChange()
    {
        // TODO - currently not supported - to support this, we will need to be able to hold a list of references o
        // all the config that has been created so far and trigger this callback when the config changes.
        // also need to compare the old and new value to make sure the content has indeed been changed
        _initialised = false;
    }

    virtual ~ConfigBase() {}

protected:
    std::string     _name;
    bool            _initialised;
    ConfigHandler&  _handler;
};



template<typename ConfigType>
class Config : public ConfigBase
{
public:
    Config(ConfigHandler& handler, const char * name, const ConfigType& defaultValue)
    : ConfigBase    (handler, name)
    , _defaultValue (defaultValue)
    , _value        (defaultValue)
    {
        _initialised = _handler.getValue<ConfigType>(name, _value);
    }

    virtual ~Config()
    {
    }

    const ConfigType& getValue()
    {
        if(!_initialised)
        {
            _initialised = _handler.getValue<ConfigType>(_name.c_str(), _value);
            if(_initialised)
            {
                return _value;
            }
            else
            {
                return _defaultValue;
            }
        }

        return _value;
    }

    const ConfigType& getDefaultValue()
    {
        return _defaultValue;
    }

private:
    ConfigType      _defaultValue;
    ConfigType      _value;
};

}  // namespace vf_common


#endif /* CONFIG_H_ */
