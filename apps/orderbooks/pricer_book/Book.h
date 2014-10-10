/*
 * Book.h
 *
 *  Created on: 21 May 2013
 *      Author: Wei Liew [wei.liew@outlook.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#ifndef BOOK_H_
#define BOOK_H_

#include <iostream>
#include <iomanip>
#include <set>
#include <cmath>

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

template<typename OrderType>
class Book
{
public:
    // in c++11, multiset order of equivalent values are stored in the order it is inserted
    // hence we can use it to store our orders and price levels
    typedef std::multiset<OrderType, BidComparator<OrderType> > BidPriceLevelType;
    typedef std::multiset<OrderType, AskComparator<OrderType> > AskPriceLevelType;

    void addOrder(OrderType& order)
    {
        if(order.side() == OrderType::BID)
        {
            _bidPriceLevel.insert(std::move(order));
        }
        else if(order.side() == OrderType::ASK)
        {
            _askPriceLevel.insert(std::move(order));
        }
        // else - report error
    }

    template<typename ...Args>
    void addBid(Args&&... args)
    {
        _bidPriceLevel.emplace(args...);
    }

    template<typename ...Args>
    void addAsk(Args&&... args)
    {
        _askPriceLevel.emplace(args...);
    }

    void modifyOrder(OrderType& order)
    {
        if(order.side() == OrderType::BID)
        {
            modifyOrder(_bidPriceLevel, order);
        }
        else if(order.side() == OrderType::ASK)
        {
            modifyOrder(_askPriceLevel, order);
        }
        // else - report error
    }

    const BidPriceLevelType& getBidPriceLevel() const
    {
        return _bidPriceLevel;
    }

    const AskPriceLevelType& getAskPriceLevel() const
    {
        return _askPriceLevel;
    }

    void print()
    {
        // function to print out the orderbook  in easy to read format
        using std::cout;
        using std::endl;
        using std::left;
        using std::right;
        using std::setw;

        cout << left << setw(8) << "BID_ID" << setw(8) << "BID" << setw(10) << "PRICE" << setw(8) << "ASK" << setw(8) << "ASK_ID" << setw(6) << "COND" << endl;

        typename AskPriceLevelType::iterator askIter = _askPriceLevel.begin();
        for(auto& bid : _bidPriceLevel)
        {
            while(askIter != _askPriceLevel.end() && bid.wholePrice() > (*askIter).wholePrice())
            {
                cout << left << setw(8) << " " << setw(8) << " ";
                cout << left << setw(10) << (*askIter).price() << setw(8) << (*askIter).size() << setw(8) << (*askIter).id() << endl;
                ++askIter;
            }

            cout << left << setw(8) << bid.id() << setw(8) << bid.size() << setw(10) << bid.price();
            if(askIter != _askPriceLevel.end() && bid.wholePrice() == (*askIter).wholePrice())
            {
                cout << left << setw(8) << (*askIter).size() << setw(8) << (*askIter).id() << setw(6) << "x" << endl;
                ++askIter;
            }
            else
            {
                cout << setw(30) << " " << endl;
            }
        }

        while(askIter != _askPriceLevel.end())
        {
            cout << left << setw(8) << " " << setw(8) << " ";
            cout << left << setw(10) << (*askIter).price() << setw(8) << (*askIter).size() << setw(8) << (*askIter).id() << endl;
            ++askIter;
        }
    }

private:
    template<typename PriceLevelType>
    void modifyOrder(PriceLevelType& pl, OrderType& order)
    {
        auto rangeFound = pl.equal_range(order);
        if(rangeFound.first == rangeFound.second)
        {
            // not found
            return;
        }

        while(rangeFound.first != rangeFound.second)
        {
            const OrderType& foundOrder = *rangeFound.first;
            if(foundOrder.matchId(order.id()))
            {
                // found the matching id - update the order
                switch(order.getAction())
                {
                case OrderType::DELETE:
                    pl.erase(rangeFound.first);
                    break; // iterator is no longer valid - but we are breaking and returning anyway

                case OrderType::REDUCE:
                    if(foundOrder.size() == order.size())
                    {
                        pl.erase(rangeFound.first);
                    }
                    else
                    {
                        foundOrder.setSize(foundOrder.size() - order.size());
                    }
                    break;

                case OrderType::ADD:
                    foundOrder.setSize(foundOrder.size() + order.size());
                    break;

                case OrderType::UPDATE:
                    foundOrder.setSize(order.size());
                    break;

                default:
                    // op-op
                    break;
                }

                // assume id is unique - i.e. dupe id is not possible in book
                break;
            }
            ++rangeFound.first;
        }
    }

    BidPriceLevelType _bidPriceLevel;
    AskPriceLevelType _askPriceLevel;
};

template<typename IdType, unsigned short PriceDecimal = 2, unsigned short SizeDecimal = 0>
class Order
{
public:
    enum OrderAction
    {
        UNDEFINED = 0,
        REDUCE,
        ADD,
        UPDATE,
        DELETE
    };

    enum Side
    {
        UNKNOWN = 0,
        BID,
        ASK
    };

    Order(const IdType& id, unsigned int price, unsigned int size, Side side)
    : _orderId(id)
    , _price(price)
    , _size(size)
    , _action(UNDEFINED)
    , _side(side)
    {
    }

    Order(const Order<IdType, PriceDecimal, SizeDecimal>& order)
    : _orderId(order._orderId)
    , _price(order._price)
    , _size(order._size)
    , _action(order._action)
    , _side(order._side)
    {

    }

    bool operator<(const Order<IdType, PriceDecimal, SizeDecimal>& rhs) const
    {
        return _price < rhs._price;
    }

    bool operator>(const Order<IdType, PriceDecimal, SizeDecimal>& rhs) const
    {
        return _price > rhs._price;
    }

    bool matchId(const IdType& id) const
    {
        return _orderId == id;
    }

    double price() const
    {
        return _price/pow(10.0,PriceDecimal);
    }

    double size() const
    {
        return _size/pow(10.0, SizeDecimal);
    }
     unsigned int wholePrice() const
     {
         return _price;
     }

     unsigned int wholeSize() const
     {
         return _size;
     }

     const IdType& id() const
     {
         return _orderId;
     }

     void setAction(OrderAction action)
     {
         _action = action;
     }

     OrderAction getAction() const
     {
         return _action;
     }

     void setSize(unsigned int size) const
     {
         _size = size;
     }

     Side side() const
     {
         return _side;
     }
private:
    IdType         _orderId;
    unsigned int   _price;
    // reason for mutable here is - we want to be able to change the size without changing the order it is stored -
    // i.e. changing this value should not have an impact on how the order is stored
    mutable unsigned int   _size;
    OrderAction    _action;
    Side           _side;
};




#endif /* BOOK_H_ */
