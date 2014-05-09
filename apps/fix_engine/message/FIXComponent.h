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

#include <boost/foreach.hpp>

namespace osf_fix
{

// Note, fix header and trailer is also classified as FIXComponent with an empty name
class FIXComponent
{
public:
    FIXComponent(const std::string& name, bool required = true, unsigned int fid = 0)
    : _name(name)
    , _groupFid(fid)      // only present if this is a group type
    , _required(required) // required should be true for components, but not necessary for groups
    {

    }

    virtual ~FIXComponent(){}

    void addField(boost::shared_ptr<FIXField> field, bool required)
    {
        _sortedFields.push_back(field);
        _isFieldRequired.push_back(required);
    }

    const char * getName()
    {
        return _name.c_str();
    }

    unsigned int getGroupId()
    {
        return _groupFid;
    }

    bool isRequired()
    {
        return _required;
    }

    const std::vector<bool>& getIsRequiredVec()
    {
        return _isFieldRequired;
    }

    const std::vector<boost::shared_ptr<FIXField> >& getSortedFields()
    {
        return _sortedFields;
    }

    void appendFields(const boost::shared_ptr<FIXComponent>& components)
    {
        int numElements = components->_isFieldRequired.size();
        for(int count=0;count<numElements;++count)
        {
            addField(components->_sortedFields[count], components->_isFieldRequired[count]);
        }
    }

    void addGroup(boost::shared_ptr<FIXComponent> group)
    {
        _groupMap.insert(std::make_pair(group->getGroupId(), group));
    }

    std::string toString(const std::string& prePend)
    {
        std::string toRet;
        BOOST_FOREACH(const boost::shared_ptr<FIXField>& field, _sortedFields)
        {
            toRet.append(prePend);
            toRet.append(field->getName());
            toRet.append("\n");
            if(field->getType() == FIXField::NUM_IN_GROUP)
            {
                // print field in group
                std::unordered_map<int, boost::shared_ptr<FIXComponent> >::iterator findIter = _groupMap.find(field->getFid());
                if(findIter != _groupMap.end())
                {
                    std::string appendStr = prePend;
                    appendStr.append("--");
                    toRet.append((*findIter).second->toString(appendStr));
                }
            }
        }

        return toRet;
    }

private:
    std::string                 _name;
    unsigned int                _groupFid;
    bool                        _required;

    std::vector<bool>                                           _isFieldRequired;
    std::vector<boost::shared_ptr<FIXField> >                   _sortedFields;
    std::unordered_map<int, boost::shared_ptr<FIXComponent> >   _groupMap;
};

}  // namespace osf_fix


#endif /* FIXCOMPONENT_H_ */
