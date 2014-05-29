/*
 * FIXComponent.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef FIXCOMPONENT_H_
#define FIXCOMPONENT_H_

#include <strstream>
#include "FIXField.h"

namespace vf_fix
{

/* FIXComponent is a general class that can either be one of the following types:
 * Component - arbitrary list of fields and groups
 * Group - a repeating group in FIX
 * Message - a message that encompass fields, components and groups
 */
class FIXComponent
{
public:
    enum ComponentType
    {
        UNKNOWN_TYPE = 0,
        COMPONENT,
        GROUP,
        MESSAGE
    };

    enum MessageCategory
    {
        UNKNOWN_CAT = 0,
        ADMIN,
        APP
    };

    FIXComponent(const std::string& name, const std::string& type)
    : _name(name)
    , _type(UNKNOWN_TYPE)
    , _msgCat(UNKNOWN_CAT)
    {
        if(!type.compare("component"))
        {
            _type = COMPONENT;
        }
        else if(!type.compare("message"))
        {
            _type = MESSAGE;
        }
        else if(!type.compare("group"))
        {
            _type = GROUP;
        }

        memset(&_msgType, 0, sizeof(_msgType));
    }

    ~FIXComponent(){}

    void addField(std::shared_ptr<FIXField> field, bool required)
    {
        _orderedFields.push_back(field);
        _isFieldRequired.push_back(required);
    }

    const char * name()
    {
        return _name.c_str();
    }

    unsigned int groupId()
    {
        return _groupSeperatorField->getFid();
    }

    std::shared_ptr<FIXField> groupSepField()
    {
        return _groupSeperatorField;
    }

    const std::vector<bool>& requiredList()
    {
        return _isFieldRequired;
    }

    const std::vector<std::shared_ptr<FIXField>>& orderedFields()
    {
        return _orderedFields;
    }

    void addGroup(std::shared_ptr<FIXComponent> group, bool required)
    {
        // create a new field for this group/component
        auto field = group->groupSepField();
        if(!field.get())
        {
            // the added component is a plain component with no fid
            field = std::make_shared<FIXField>(0, group->name(), "component");
        }

        field->setComponentField(group);
        addField(field, required);
    }

    std::ostream& dump(std::ostream& oss) const
    {
        std::string hdr = "Component";
        if(_type == GROUP)
        {
            hdr = "Group";
        }
        else if(_type == MESSAGE)
        {
            hdr = "Message";
        }
        else if(_type != COMPONENT)
        {
            hdr = "Unknown";
        }
        oss << hdr << " name: " << _name << " type: " << _type << std::endl;
        for_each(_orderedFields.begin(), _orderedFields.end(), [this, &oss](std::shared_ptr<FIXField> field){
            oss << "    " << *field << std::endl;
        });

        return oss;
    }

    // GROUP type only
    void setGroupSepField(std::shared_ptr<FIXField> field)
    {
        _groupSeperatorField = field;
    }

    ComponentType type()
    {
        return _type;
    }

    MessageCategory msgCat()
    {
        return _msgCat;
    }

    char* msgType()
    {
        return _msgType;
    }

    void setMsgCat(const std::string& cat)
    {
        if(cat == "admin")
        {
            setMsgCat(ADMIN);
        }
        else if(cat == "app")
        {
            setMsgCat(APP);
        }
    }

    void setMsgCat(MessageCategory cat)
    {
        _msgCat = cat;
    }

    void setMsgType(const char* msgType)
    {
        if(!msgType)
        {
            return;
        }
        memcpy(&_msgType, msgType, sizeof(_msgType));
    }

private:
    std::string                                             _name;
    ComponentType                                           _type;

    // TODO - how to have an ordered list of both fields and components/groups ?? - i.e. add the group sep field into the field list ??
    std::vector<bool>                                       _isFieldRequired;
    std::vector<std::shared_ptr<FIXField>>                  _orderedFields;

    // type specific members
    std::shared_ptr<FIXField>                               _groupSeperatorField;   // GROUP type only
    MessageCategory                                         _msgCat;                // MESSAGE type only
    char                                                    _msgType[2];            // MESSAGE type only
};

} // namespace vf_fix

std::ostream& operator<<(std::ostream& oss, const vf_fix::FIXComponent& comp)
{
    return comp.dump(oss);
}

#endif /* FIXCOMPONENT_H_ */
