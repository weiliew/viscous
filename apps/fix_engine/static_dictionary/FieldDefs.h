/*
 * FieldDefs.h
 *
 *  Created on: 16 Jun 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef FIELDDEFS_H_
#define FIELDDEFS_H_

#include "utilities/StringConstant.h"
#include "SFIXFields.h"
#include "SFIXGroups.h"

using namespace vf_common;

namespace vf_fix
{

StringConstant PosAmtType("PosAmtType");
StringConstant PosAmt("PosAmt");
StringConstant PositionCurrency("PositionCurrency");
StringConstant NoPosAmt("NoPosAmt");

StringConstant STRING("STRING");
StringConstant AMT("AMT");




StringConstant ArrEnum_707[] = {StringConstant("CASH"), StringConstant("CRES"), StringConstant("FMTM"),
                                StringConstant("IMTM"), StringConstant("PREM"), StringConstant("SMTM"),
                                StringConstant("TVAR"), StringConstant("VADJ"), StringConstant("SETL")};

StringConstantArr StrArrEnum_707(ArrEnum_707);

StringConstant ArrDesc_707[] = {StringConstant("CASH_AMOUNT"), StringConstant("CASH_RESIDUAL_AMOUNT"),
                                StringConstant("FINAL_MARK_TO_MARKET_AMOUNT"), StringConstant("INCREMENTAL_MARK_TO_MARKET_AMOUNT"),
                                StringConstant("PREMIUM_AMOUNT"), StringConstant("START_OF_DAY_MARK_TO_MARKET_AMOUNT"),
                                StringConstant("TRADE_VARIATION_AMOUNT"), StringConstant("VALUE_ADJUSTED_AMOUNT"),
                                StringConstant("SETTLEMENT_VALUE")};

StringConstantArr StrArrDesc_707(ArrDesc_707);

StringConstantArr EmptyArray;


template<typename Required, typename Validate>
using SFIXField_707 = SFIXField<707,
                  PosAmtType,
                  STRING,
                  StrArrEnum_707,
                  StrArrDesc_707,
                  std::false_type,
                  Required,
                  Validate>;

template<typename Required, typename Validate>
using SFIXField_708 = SFIXField<708,
                    PosAmt,
                    AMT,
                    EmptyArray,
                    EmptyArray,
                    std::false_type,
                    Required,
                    Validate>;

template<typename Required, typename Validate>
using SFIXField_1055 = SFIXField<1055,
                  PositionCurrency,
                  STRING,
                  EmptyArray,
                  EmptyArray,
                  std::false_type,
                  Required,
                  Validate>;


template<typename Required, typename Validate, unsigned int Capacity>
using SFIXGroup_753 = SFIXGroup<753,
                NoPosAmt,
                Required,
                Validate,
                Capacity,
                SFIXField_707<std::false_type, Validate>,
                SFIXField_708<std::false_type, Validate>,
                SFIXField_1055<std::false_type, Validate>>;

}

#endif /* FIELDDEFS_H_ */
