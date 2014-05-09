/*
 * FIXField.h
 *
 *  Created on: 26 Apr 2014
 *      Author: Wei Liew [wei@onesixeightsolutuons.comm]
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef FIXFIELD_H_
#define FIXFIELD_H_

#include <boost/algorithm/string.hpp>

using namespace osf_common;

namespace osf_fix
{

class FIXField
{
public:
    enum FieldType
    {
        INT = 0,
        LENGTH,
        TAG_NUM,
        SEQ_NUM,
        DAY_OF_MONTH,
        NUM_IN_GROUP, // group separator is always an integer
        FLOAT = 100,
        QTY,
        PRICE,
        PRICE_OFFSET,
        AMT,
        PERCENTAGE,
        CHAR = 200,
        BOOLEAN,
        STRING = 300,
        MULTIPLE_CHAR_VALUE,
        MULTIPLE_STRING_VALUE,
        COUNTRY,
        CURRENCY,
        EXCHANGE,
        MONTH_YEAR,
        UTC_TIMESTAMP,
        UTC_TIME_ONLY,
        UTC_DATE_ONLY,
        LOCAL_MKT_DATE,
        TZ_TIME_ONLY,
        TZ_TIMESTAMP,
        DATA,
        XML_DATA,
        LANGUAGE,
        PATTERN = 400,
        TENOR,
        RESERVED_100_PLUS,
        RESERVED_1000_PLUS,
        RESERVED_4000_PLUS,
        UNKNOWN_TYPE
    };

    FIXField(unsigned int fid, const std::string& name, const std::string& type)
    : _fid(fid)
    , _name(name)
    , _type(UNKNOWN_TYPE)
    , _typeName(type)
    {
        // here's the slow bit
        std::string lowerType = type;
        boost::algorithm::to_lower(lowerType);
        if(!lowerType.compare("int"))
        {
            _type = INT;
        }
        else if(!lowerType.compare("length"))
        {
            _type = LENGTH;
        }
        else if(!lowerType.compare("tagnum"))
        {
            _type = TAG_NUM;
        }
        else if(!lowerType.compare("seqnum"))
        {
            _type = SEQ_NUM;
        }
        else if(!lowerType.compare("numingroup"))
        {
            _type = NUM_IN_GROUP;
        }
        else if(!lowerType.compare("dayofmonth"))
        {
            _type = DAY_OF_MONTH;
        }
        else if(!lowerType.compare("float"))
        {
            _type = FLOAT;
        }
        else if(!lowerType.compare("qty"))
        {
            _type = QTY;
        }
        else if(!lowerType.compare("price"))
        {
            _type = PRICE_OFFSET;
        }
        else if(!lowerType.compare("amt"))
        {
            _type = AMT;
        }
        else if(!lowerType.compare("percentage"))
        {
            _type = PERCENTAGE;
        }
        else if(!lowerType.compare("char"))
        {
            _type = CHAR;
        }
        else if(!lowerType.compare("boolean"))
        {
            _type = BOOLEAN;
        }
        else if(!lowerType.compare("string"))
        {
            _type = STRING;
        }
        else if(!lowerType.compare("multiplecharvalue"))
        {
            _type = MULTIPLE_CHAR_VALUE;
        }
        else if(!lowerType.compare("multiplestringvalue"))
        {
            _type = MULTIPLE_STRING_VALUE;
        }
        else if(!lowerType.compare("country"))
        {
            _type = COUNTRY;
        }
        else if(!lowerType.compare("currency"))
        {
            _type = CURRENCY;
        }
        else if(!lowerType.compare("exchange"))
        {
            _type = EXCHANGE;
        }
        else if(!lowerType.compare("monthyear"))
        {
            _type = MONTH_YEAR;
        }
        else if(!lowerType.compare("utctimestamp"))
        {
            _type = UTC_TIMESTAMP;
        }
        else if(!lowerType.compare("utctimeonly"))
        {
            _type = UTC_TIME_ONLY;
        }
        else if(!lowerType.compare("utcdateonly"))
        {
            _type = UTC_DATE_ONLY;
        }
        else if(!lowerType.compare("localmktdate"))
        {
            _type = LOCAL_MKT_DATE;
        }
        else if(!lowerType.compare("tztimeonly"))
        {
            _type = TZ_TIME_ONLY;
        }
        else if(!lowerType.compare("tztimestamp"))
        {
            _type = TZ_TIMESTAMP;
        }
        else if(!lowerType.compare("data"))
        {
            _type = DATA;
        }
        else if(!lowerType.compare("xmldata"))
        {
            _type = XML_DATA;
        }
        else if(!lowerType.compare("language"))
        {
            _type = LANGUAGE;
        }
        else if(!lowerType.compare("pattern"))
        {
            _type = PATTERN;
        }
        else if(!lowerType.compare("tenor"))
        {
            _type = TENOR;
        }
        else if(!lowerType.compare("reserved100plus"))
        {
            _type = RESERVED_100_PLUS;
        }
        else if(!lowerType.compare("reserved1000plus"))
        {
            _type = RESERVED_1000_PLUS;
        }
        else if(!lowerType.compare("reserved4000plus"))
        {
            _type = RESERVED_4000_PLUS;
        }
    }

    virtual ~FIXField(){}

    unsigned int getFid()
    {
        return _fid;
    }

    const char * getName()
    {
        return _name.c_str();
    }

    FieldType getType()
    {
        return _type;
    }

    const char * getTypeName()
    {
        return _typeName.c_str();
    }

private:
    unsigned int    _fid;
    std::string     _name;
    FieldType       _type;
    std::string     _typeName;
};

}  // namespace osf_fix


#endif /* FIXFIELD_H_ */
