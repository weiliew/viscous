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
#include <strstream>
#include <unordered_map>
#include "utilities/HashUtils.h"
#include "logging/Log.h"
#include "FIXField.h"
#include "FIXComponent.h"

using vf_common::str_hash;
using vf_common::str_eq;
using boost::property_tree::ptree;

namespace vf_fix
{

struct FIXDictionary_exception: virtual std::exception, virtual boost::exception { };
struct FIXField_exception:      virtual FIXDictionary_exception { };
struct FIXComponent_exception:  virtual FIXField_exception { };

typedef boost::error_info<struct tag_errno_fid,int> errno_fid;
typedef boost::error_info<struct tag_errno_name,int> errno_name;
typedef boost::error_info<struct tag_errno_desc,std::string> errno_desc;

template<typename Logger>
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

    ~FIXDictionary()
    {
    }

    bool load(const std::string& filename, const std::string& transportFilename = "")
    {
        try
        {
            if(!transportFilename.empty())
            {
                boost::property_tree::read_xml(filename.c_str(), _ptreeTransport);

                // get the root container and its attributes
                ptree fixNode = _ptreeTransport.get_child("fix");

                _versionTransportMajor = _ptreeTransport.get<unsigned int>("fix.<xmlattr>.major");
                _versionTransportMinor = _ptreeTransport.get<unsigned int>("fix.<xmlattr>.minor");
                _versionTransportServicePack = _ptreeTransport.get<unsigned int>("fix.<xmlattr>.servicepack");
                _versionTransportType = _ptreeTransport.get<std::string>("fix.<xmlattr>.type");

                // parse the field definitions
                parseFieldList(fixNode.get_child("fields"));

                // parse the component definitions
                parseComponentList(fixNode.get_child("components"));

                // parse the headers, messages and trailers
                _header = parseComponent("component", fixNode.get_child("header"), "header");
                _trailer = parseComponent("component", fixNode.get_child("trailer"), "trailer");
            }
        }
        catch(boost::exception& except)
        {
            VF_LOG_WARN(_logger, "Failed to load data dictionary [" << transportFilename << "] due to exception [" << boost::diagnostic_information(except) << "].");
            return false;
        }
        catch(std::exception& except)
        {
            VF_LOG_WARN(_logger, "Failed to load data dictionary [" << transportFilename << "] due to exception [" << except.what() << "].");
            return false;
        }
        catch(...)
        {
            // unknown exception
            VF_LOG_WARN(_logger, "Failed to load data dictionary [" << transportFilename << "] due to unknown exception.");
            return false;
        }

        try
        {
            // parses the fix data dictionary
            boost::property_tree::read_xml(filename.c_str(), _ptree);

            // get the root container and its attributes
            ptree fixNode = _ptree.get_child("fix");

            _versionMajor = _ptree.get<unsigned int>("fix.<xmlattr>.major");
            _versionMinor = _ptree.get<unsigned int>("fix.<xmlattr>.minor");
            _versionServicePack = _ptree.get<unsigned int>("fix.<xmlattr>.servicepack");
            _versionType = _ptree.get<std::string>("fix.<xmlattr>.type");

            if(transportFilename.empty())
            {
                // parse the field definitions
                parseFieldList(fixNode.get_child("fields"));

                // parse the component definitions
                parseComponentList(fixNode.get_child("components"));

                // parse the headers, messages and trailers
                _header = parseComponent("component", fixNode.get_child("header"), "header");
                _trailer = parseComponent("component", fixNode.get_child("trailer"), "trailer");
            }

            // parse all the message types
            parseComponentList(fixNode.get_child("messages"));

        }
        catch(boost::exception& except)
        {
            VF_LOG_WARN(_logger, "Failed to load data dictionary [" << filename << "] due to exception [" << boost::diagnostic_information(except) << "].");
            return false;
        }
        catch(std::exception& except)
        {
            VF_LOG_WARN(_logger, "Failed to load data dictionary [" << filename << "] due to exception [" << except.what() << "].");
            return false;
        }
        catch(...)
        {
            // unknown exception
            VF_LOG_WARN(_logger, "Failed to load data dictionary [" << filename << "] due to unknown exception.");
            return false;
        }

        _parsed = true;

        return _parsed;
    }

    bool parsed()
    {
        return _parsed;
    }

    unsigned int getFID(const char * name)
    {
        auto findIter = _fieldStringMap.find(name);
        if(findIter != _fieldStringMap.end())
        {
            return (*findIter).second->getFid();
        }

        return 0;
    }

    std::shared_ptr<FIXField> getField(const char * name)
    {
        auto findIter = _fieldStringMap.find(name);
        if(findIter != _fieldStringMap.end())
        {
            return (*findIter).second;
        }

        return nullptr;
    }

    std::shared_ptr<FIXField> getField(unsigned int fid)
    {
        auto findIter = _fieldMap.find(fid);
        if(findIter != _fieldMap.end())
        {
            return (*findIter).second;
        }

        return nullptr;
    }

    std::shared_ptr<FIXComponent> getComponent(const char * name)
    {
        auto findIter = _componentMap.find(name);
        if(findIter != _componentMap.end())
        {
            return (*findIter).second;
        }

        return nullptr;
    }

    std::shared_ptr<FIXComponent> getGroup(unsigned int groupId)
    {
        auto findIter = _groupMap.find(groupId);
        if(findIter != _groupMap.end())
        {
            return (*findIter).second;
        }

        return nullptr;
    }

    std::ostream& dump(std::ostream& oss) const
    {
        oss << "\nHEADER\n" << "  " << *_header << std::endl;
        oss << "TRAILER\n"  << "  " << *_trailer << std::endl;
        oss << "FIELDS\n";

        for_each(_fieldMap.begin(), _fieldMap.end(), [&oss](std::pair<unsigned int, std::shared_ptr<FIXField>> fieldPair){
            oss << "  " << *fieldPair.second << std::endl;
        });

        oss << "GROUP\n";
        for_each(_groupMap.begin(), _groupMap.end(), [&oss](std::pair<unsigned int, std::shared_ptr<FIXComponent>> componentPair) {
            oss << "  "<< *componentPair.second << std::endl;
        });
        oss << "COMPONENT\n";
        for_each(_componentMap.begin(), _componentMap.end(), [&oss](std::pair<const char *, std::shared_ptr<FIXComponent>> componentPair) {
            oss << "  "<< *componentPair.second << std::endl;
        });
        oss << "MSGTYPE\n";
        for_each(_msgTypeMap.begin(), _msgTypeMap.end(), [&oss](std::pair<const char *, std::shared_ptr<FIXComponent>> componentPair) {
            oss << "  "<< *componentPair.second << std::endl;
        });

        return oss;
    }

private:
    void parseFieldList(const ptree& node)
    {
        std::for_each(node.begin(), node.end(), [this](ptree::value_type const& child){
            // make sure this is a field node
            if((child.first.compare("field")))
            {
                BOOST_THROW_EXCEPTION(FIXField_exception() << errno_desc("cannot find node with name 'field'"));
            }

            try
            {
                parseField(child.second);
            }
            catch(boost::exception& except)
            {
                BOOST_THROW_EXCEPTION(FIXField_exception() <<  errno_desc("error parsing field list") << errno_desc(boost::current_exception_diagnostic_information()));
            }
        });
    }

    void parseField(const ptree& field)
    {
        unsigned int fid = field.get<unsigned int>("<xmlattr>.number");
        std::shared_ptr<FIXField> newField = std::make_shared<FIXField>(fid, field.get<std::string>("<xmlattr>.name"), field.get<std::string>("<xmlattr>.type"));

        _fieldMap.insert(std::make_pair(fid, newField));
        _fieldStringMap.insert(std::make_pair(newField->getName(), newField));

        // check if this is an enum type
        boost::optional<const ptree&> valueList = field.get_child_optional("value");
        if(!valueList)
        {
            return;
        }
        std::for_each(field.begin(), field.end(), [&newField](ptree::value_type const& node){
            if(node.first == "value")
            {
                newField->addEnumVal(node.second.get<std::string>("<xmlattr>.enum"), node.second.get<std::string>("<xmlattr>.description"));
            }
        });
    }

    void parseComponentList(const ptree& node)
    {
        std::for_each(node.begin(), node.end(), [this](ptree::value_type const& child){
            if(child.first.compare("component") && child.first.compare("message"))
            {
                BOOST_THROW_EXCEPTION(FIXComponent_exception() << errno_desc("encountered unrecognised component/message node with with name" + child.first));
            }

            try
            {
                parseComponent(child.first, child.second);
            }
            catch(boost::exception& except)
            {
                BOOST_THROW_EXCEPTION(FIXComponent_exception() <<  errno_desc("error parsing component list")
                        << errno_desc(boost::current_exception_diagnostic_information()));
            }
        });
    }

    std::shared_ptr<FIXComponent> parseComponent(const std::string& type, const ptree& node, const char * name = NULL)
    {
        // component definitions - each component can contain a list of fields, as well
        // as repeating group of field
        std::string compName = "";
        if(name)
        {
            compName = name;
        }
        else
        {
            compName = node.get<std::string>("<xmlattr>.name").data();
        }

        auto comp = std::make_shared<FIXComponent>(compName, type);

        // if this is a group node
        if(comp->type() == FIXComponent::GROUP)
        {
            auto field = getField(compName.c_str());
            if(!field.get())
            {
                BOOST_THROW_EXCEPTION(FIXComponent_exception() <<  errno_desc(std::string("error parsing group. Group field id not found: ") +
                        compName) << errno_desc(boost::current_exception_diagnostic_information()));
            }
            comp->setGroupSepField(field);

            // add this component to the group map for tracking
            _groupMap.insert(std::make_pair(comp->groupId(), comp));
        }
        else if(comp->type() == FIXComponent::MESSAGE)
        {
            // get msg type and category
            comp->setMsgType(node.get<std::string>("<xmlattr>.msgtype").c_str());
            comp->setMsgCat(node.get<std::string>("<xmlattr>.msgcat"));

            // add this to the message map to be tracked
            _msgTypeMap.insert(std::make_pair(comp->msgType(), comp));
        }
        else
        {
            // add this to the component map to be tracked
            _componentMap.insert(std::make_pair(comp->name(), comp));
        }

        // add fields and groups
        std::for_each(node.begin(), node.end(), [this, &comp](ptree::value_type const& child){
            // if child is a group/component node, parse group recursively
            if(!child.first.compare("group") || !child.first.compare("component"))
            {
                bool required = child.second.get<std::string>("<xmlattr>.required") == "Y";
                // parse group
                comp->addGroup(parseComponent(child.first, child.second), required);
            }
            else if(!child.first.compare("field"))
            {
                bool required = child.second.get<std::string>("<xmlattr>.required") == "Y";
                // field definition must already exist by this stage
                auto field = getField(child.second.get<std::string>("<xmlattr>.name").c_str());
                if(!field.get())
                {
                    BOOST_THROW_EXCEPTION(FIXComponent_exception() << errno_desc(std::string("field for component") +  comp->name() + " not found: " +
                            child.second.get<std::string>("<xmlattr>.name")));
                }
                comp->addField(field, required);
            }
            // else - attributes that can be skipped
        });

        return comp;
    }

    Logger                      _logger;
    boost::property_tree::ptree _ptree;
    boost::property_tree::ptree _ptreeTransport;

    bool                        _parsed;

    unsigned int                _versionMajor;
    unsigned int                _versionMinor;
    unsigned int                _versionServicePack;
    std::string                 _versionType;

    unsigned int                _versionTransportMajor;
    unsigned int                _versionTransportMinor;
    unsigned int                _versionTransportServicePack;
    std::string                 _versionTransportType;

    std::shared_ptr<FIXComponent> _header;
    std::shared_ptr<FIXComponent> _trailer;

    std::unordered_map<unsigned int, std::shared_ptr<FIXField> >                      _fieldMap;          // fid num to field map
    std::unordered_map<const char *, std::shared_ptr<FIXField>, str_hash, str_eq>     _fieldStringMap;    // fid name to field map
    std::unordered_map<unsigned int, std::shared_ptr<FIXComponent>>                   _groupMap;          // group id to repeating group map
    std::unordered_map<const char *, std::shared_ptr<FIXComponent>, str_hash, str_eq> _componentMap;      // component name to component map
    std::unordered_map<const char *, std::shared_ptr<FIXComponent>, str_hash, str_eq> _msgTypeMap;        // TODO - instead of const char * - can be more efficient as we knoe msg type is 2 char
};

}  // namespace vf_fix

template<typename Logger>
std::ostream& operator<<(std::ostream& oss, const vf_fix::FIXDictionary<Logger>& dict)
{
    return dict.dump(oss);
}

#endif /* FIXDICTIONARY_H_ */
