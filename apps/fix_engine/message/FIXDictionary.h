/*
 * FIXDictionary.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef FIXDICTIONARY_H_
#define FIXDICTIONARY_H_

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/exception/all.hpp>

#include <unordered_map>
#include "utilities/HashUtils.h"
#include "logging/Log.h"
#include "FIXField.h"
#include "FIXMessage.h"
#include "FIXComponent.h"

namespace osf_fix
{

struct FIXDictionary_exception: virtual std::exception, virtual boost::exception { };
struct FIXField_exception: virtual FIXDictionary_exception { };
struct FIXComponent_exception: virtual FIXField_exception { };
struct FIXHeader_exception: virtual FIXField_exception { };

typedef boost::error_info<struct tag_errno_fid,int> errno_fid;
typedef boost::error_info<struct tag_errno_name,int> errno_name;
typedef boost::error_info<struct tag_errno_desc,std::string> errno_desc;



class FIXDictionary
{
public:
    FIXDictionary(Logger& logger)
    : _logger(logger)
    , _parsed(false)
    , _versionMajor(0)
    , _versionMinor(0)
    , _versionServicePack(0)
    {

    }

    virtual ~FIXDictionary()
    {

    }

    bool loadDictionary(const char * filename);

    /*
    boost::shared_ptr<FIXMessage> getFIXMessage(const char * msgType)
    {
        // This function creates and returns a FIX message of the required type based on the data dictionary

    }
    */

    bool isParsed()
    {
        return _parsed;
    }

    unsigned int getFieldID(const char * name)
    {
        std::unordered_map<const char *, boost::shared_ptr<FIXField>, str_hash, str_eq>::iterator findIter = _fieldStringMap.find(name);
        if(findIter != _fieldStringMap.end())
        {
            return (*findIter).second->getFid();
        }

        return 0;
    }

    bool getField(const char * name, boost::shared_ptr<FIXField>& ret)
    {
        std::unordered_map<const char *, boost::shared_ptr<FIXField>, str_hash, str_eq>::iterator findIter = _fieldStringMap.find(name);
        if(findIter != _fieldStringMap.end())
        {
            ret = (*findIter).second;
            return true;
        }

        return false;
    }

    bool getComponent(const char * name, boost::shared_ptr<FIXComponent>& component)
    {
        std::unordered_map<const char *, boost::shared_ptr<FIXComponent>, str_hash, str_eq>::iterator findIter = _componentMap.find(name);
        if(findIter != _componentMap.end())
        {
            component = (*findIter).second;
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    void parseFields    (const boost::property_tree::ptree& node);
    void parseField     (const boost::property_tree::ptree& field);
    void parseComponents(const boost::property_tree::ptree& node);
    boost::shared_ptr<FIXComponent> parseComponent(const boost::property_tree::ptree& component, const std::string& name, unsigned int groupFid = 0, bool required = false);

    Logger                      _logger;
    boost::property_tree::ptree _ptree;
    bool                        _parsed;

    unsigned int                _versionMajor;
    unsigned int                _versionMinor;
    unsigned int                _versionServicePack;
    std::string                 _versionType;

    boost::shared_ptr<FIXComponent> _header;
    boost::shared_ptr<FIXComponent> _trailer;

    std::unordered_map<unsigned int, boost::shared_ptr<FIXField> >                      _fieldMap;
    std::unordered_map<const char *, boost::shared_ptr<FIXField>, str_hash, str_eq>     _fieldStringMap;
    std::unordered_map<const char *, boost::shared_ptr<FIXComponent>, str_hash, str_eq> _componentMap;
};

}  // namespace osf_fix


#endif /* FIXDICTIONARY_H_ */
