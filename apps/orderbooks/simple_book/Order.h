/*
 * Order.h
 *
 *  Created on: 6 Oct 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef ORDER_H_
#define ORDER_H_

struct Order
{
    enum
    {
        ASK = 'O',
        BID = 'B'
    };

    Order()
    : _id(0)
    , _side(ASK)
    , _price(0.0)
    , _quantity(0)
    {
    }

    Order(int id, char side, double price, int quantity)
    : _id(id)
    , _side(side)
    , _price(price)
    , _quantity(quantity)
    {
    }

    Order(const Order& rhs)
    : _id(rhs._id)
    , _side(rhs._side)
    , _price(rhs._price)
    , _quantity(rhs._quantity)
    {
    }

    const Order& operator=(const Order& rhs)
    {
        _id = rhs._id;
        _side = rhs._side;
        _price = rhs._price;
        _quantity = rhs._quantity;
        return *this;
    }

    bool operator<(const Order& rhs) const
    {
        return _price < rhs._price;
    }

    bool operator>(const Order& rhs) const
    {
        return _price > rhs._price;
    }

    int    _id;
    char   _side;
    double _price;
    mutable int    _quantity;
};




#endif /* ORDER_H_ */
