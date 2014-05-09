/*
 * FIXDictionary.cpp
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifdef DISABLED

#include <boost/foreach.hpp>
#include "FIXDictionary.h"

using namespace osf_fix;

using boost::property_tree::ptree;

bool FIXDictionary::loadDictionary(const char * filename)
{
    if(!filename)
    {
        return false;
    }

    try
    {
        // parses the fix data dictionary
        boost::property_tree::read_xml(filename, _ptree);

        // get the root container and its attributes
        ptree fixNode = _ptree.get_child("fix");

        _versionMajor = _ptree.get<unsigned int>("fix.<xmlattr>.major");
        _versionMinor = _ptree.get<unsigned int>("fix.<xmlattr>.minor");
        _versionServicePack = _ptree.get<unsigned int>("fix.<xmlattr>.servicepack");
        _versionType = _ptree.get<std::string>("fix.<xmlattr>.type");

        // parse the field definitions
        parseFields(fixNode.get_child("fields"));

        // parse the component definitions
        parseComponents(fixNode.get_child("components"));

        // parse the headers, messages and trailers
        _header = parseComponent(fixNode.get_child("header"), "header");
        _trailer = parseComponent(fixNode.get_child("trailer"), "trailer");
    }
    catch(boost::exception& except)
    {
        OSF_LOG_WARN(_logger, "Failed to load data dictionary [" << filename << "] due to exception [" << boost::diagnostic_information(except) << "].");
        return false;
    }
    catch(std::exception& except)
    {
        OSF_LOG_WARN(_logger, "Failed to load data dictionary [" << filename << "] due to exception [" << except.what() << "].");
        return false;
    }
    catch(...)
    {
        // unknown exception
        OSF_LOG_WARN(_logger, "Failed to load data dictionary [" << filename << "] due to unknown exception.");
        return false;
    }

    _parsed = true;

    return _parsed;
}

void FIXDictionary::parseFields(const ptree& node)
{
    BOOST_FOREACH(ptree::value_type const& child, node)
    {
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
    }

    typedef std::pair<unsigned int, boost::shared_ptr<FIXField> > FieldMapEntryType;
    BOOST_FOREACH(const FieldMapEntryType& pair, _fieldMap)
    {
        OSF_LOG_INFO(_logger, "Loaded field with FID [" << pair.first << "] with name [" << pair.second->getName() << "]");
    }
}

void FIXDictionary::parseField(const ptree& field)
{
    unsigned int fid = field.get<unsigned int>("<xmlattr>.number");
    boost::shared_ptr<FIXField> newField(new FIXField(fid, field.get<std::string>("<xmlattr>.name"), field.get<std::string>("<xmlattr>.type")));
    _fieldMap.insert(std::make_pair(fid, newField));
    _fieldStringMap.insert(std::make_pair(newField->getName(), newField));
}

void FIXDictionary::parseComponents(const ptree& node)
{
    BOOST_FOREACH(ptree::value_type const& child, node)
    {
        // make sure this is a field node
        if((child.first.compare("component")))
        {
            BOOST_THROW_EXCEPTION(FIXComponent_exception() << errno_desc("encountered unrecognised component node with with name" + child.first));
        }

        try
        {
            boost::shared_ptr<FIXComponent> componentToInsert = parseComponent(child.second, child.second.get_child("<xmlattr>.name").data());
            _componentMap.insert(std::make_pair(componentToInsert->getName(), componentToInsert));
        }
        catch(boost::exception& except)
        {
            BOOST_THROW_EXCEPTION(FIXComponent_exception() <<  errno_desc("error parsing component list") << errno_desc(boost::current_exception_diagnostic_information()));
        }
    }

    typedef std::pair<const char *, boost::shared_ptr<FIXComponent> > ComponentMapEntryType;
    BOOST_FOREACH(const ComponentMapEntryType& pair, _componentMap)
    {
        OSF_LOG_INFO(_logger, "Loaded component [" << pair.first << "] with fields:\n" << pair.second->toString(std::string("  ")));
    }

}

boost::shared_ptr<FIXComponent> FIXDictionary::parseComponent(const ptree& component, const std::string& name, unsigned int groupFid, bool required)
{
    // check if self is a component or a group
    boost::shared_ptr<FIXComponent> newComponent;
    if(groupFid)
    {
        // this is a group. get the group fid
        newComponent = boost::shared_ptr<FIXComponent>(new FIXComponent(name, required, groupFid));
    }
    else
    {
        newComponent = boost::shared_ptr<FIXComponent>(new FIXComponent(name));
    }

    // add the fields into the component object
    BOOST_FOREACH(ptree::value_type const& child, component)
    {
        if(!child.first.compare("field") || !(child.first.compare("group")))
        {
            boost::shared_ptr<FIXField> storedField;
            std::string fieldName = child.second.get_child("<xmlattr>.name").data();
            char required = child.second.get<char>("<xmlattr>.required");

            // field node - get from the field list
            if(!getField(fieldName.c_str(), storedField))
            {
                std::string errStr = "encountered unrecognised field in component/group with with name ";
                errStr.append(fieldName);
                BOOST_THROW_EXCEPTION(FIXField_exception() << errno_desc(errStr));
            }
            // add the field to the component
            newComponent->addField(storedField, (required == 'Y' || required == 'y'));

            // process group if necessary
            if(!(child.first.compare("group")))
            {
                newComponent->addGroup(parseComponent(child.second, fieldName, getFieldID(fieldName.c_str()), (required == 'y' || required == 'Y')));
            }
        }
        else if((!child.first.compare("component")))
        {
            std::string fieldName = child.second.get_child("<xmlattr>.name").data();
            char required = child.second.get<char>("<xmlattr>.required");

            // check if the component is already defined
            boost::shared_ptr<FIXComponent> existingComponent;
            if(getComponent(fieldName.c_str(), existingComponent))
            {
                // add the fields of the existing component - i.e. flatten the component
                newComponent->appendFields(existingComponent);
            }
            else
            {
                // TODO - possible that the component is defined further down the file ...
                OSF_LOG_WARN(_logger, "Encountered unrecognised component name " << fieldName << " specified in component: " << newComponent->getName());
/*
                std::string errStr = "encountered unrecognised component name ";
                errStr.append(fieldName);
                BOOST_THROW_EXCEPTION(FIXField_exception() << errno_desc(errStr));
                */
            }
        }
    }

    return newComponent;
}
#endif

