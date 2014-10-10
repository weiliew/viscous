/*
 * Comparator.h
 *
 *  Created on: 6 Oct 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef COMPARATOR_H_
#define COMPARATOR_H_


// comparators used in our book containers
template<typename OrderType>
struct BidComparator
{
public:
    bool operator()(const OrderType& lhs, const OrderType& rhs)
    {
        // for bids, highest price is at the top of the list
        // i.e. lhs < rhs if price of lhs is greater than price of rhs
        return lhs > rhs;
    }
};

template<typename OrderType>
struct AskComparator
{
public:
    bool operator()(const OrderType& lhs, const OrderType& rhs)
    {
        // for asks, lowest price is at the top of the list
        // i.e. lhs < rhs if price of lhs is less than price of rhs
        return lhs < rhs;
    }
};


#endif /* COMPARATOR_H_ */
