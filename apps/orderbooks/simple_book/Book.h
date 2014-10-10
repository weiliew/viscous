/*
 * Book.h
 *
 *  Created on: 6 Oct 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include "Comparator.h"
#include <unordered_map>
#include <limits>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <set>
#include <algorithm>

#ifndef BOOK_H_
#define BOOK_H_

template<typename OrderType>
class Book
{
public:
    typedef std::multiset<OrderType, BidComparator<OrderType>>   BidPiceLevelType;
    typedef std::multiset<OrderType, AskComparator<OrderType>>   AskPiceLevelType;

    Book()
    {
    }

    virtual ~Book()
    {
    }

    void add(OrderType& order)
    {
        if(order._side == OrderType::BID)
        {
            addOrder(order, _bidPriceLevel);
        }
        else if(order._side == OrderType::ASK)
        {
            addOrder(order, _askPriceLevel);
        }
        // else - report error
    }

    void modify(OrderType& order)
    {
        if(order._side == OrderType::BID)
        {
            modifyOrder(order, _bidPriceLevel);
        }
        else if(order._side == OrderType::ASK)
        {
            modifyOrder(order, _askPriceLevel);
        }
        // else - report error
    }

    void modify(int id, int quantity)
    {
        // we have to search both the bid as well as the ask PL here
        // due to the way we are storing the orders on bid/ask levels
        // we could create a hash map to store is to order type to speed things up
        if(!modifyOrder(id, quantity, _bidPriceLevel))
        {
            modifyOrder(id, quantity, _askPriceLevel);
        }
    }

    void remove(int id)
    {
        // we have to search both the bid as well as the ask PL here
        // due to the way we are storing the orders on bid/ask levels
        // we could create a hash map to store is to order type to speed things up
        if(!removeOrder(id, _bidPriceLevel))
        {
            removeOrder(id, _askPriceLevel);
        }
    }

    double getPrice(int level, char side)
    {
        if(side == OrderType::BID)
        {
            return getPrice(level, _bidPriceLevel);
        }
        else if(side == OrderType::ASK)
        {
            return getPrice(level, _askPriceLevel);
        }
        // else - report error
        return 0.0;
    }

    int getSize(int level, char side)
    {
        if(side == OrderType::BID)
        {
            return getSize(level, _bidPriceLevel);
        }
        else if(side == OrderType::ASK)
        {
            return getSize(level, _askPriceLevel);
        }
        // else - report error
        return 0;
    }

    auto begin(char side)
    {
        if(side == OrderType::BID)
        {
            return _bidPriceLevel.begin();
        }
        else if(side == OrderType::ASK)
        {
            return _askPriceLevel.begin();
        }
        // else - report error
        return _bidPriceLevel.end();
    }

    auto end(char side)
    {
        if(side == OrderType::BID)
        {
            return _bidPriceLevel.end();
        }
        else if(side == OrderType::ASK)
        {
            return _askPriceLevel.end();
        }
        // else - report error
        return _bidPriceLevel.end();
    }

    void printSide(char side)
    {
        std::for_each(begin(side), end(side), [](const OrderType& order){
            std::cout << order._id << " " << order._price << " " << order._quantity << std::endl;
        });
    }

    void print()
    {
        // function to print out the orderbook  in easy to read format
        using std::cout;
        using std::endl;
        using std::left;
        using std::right;
        using std::setw;

        auto askIter = _askPriceLevel.begin();

        cout << left << setw(8) << "BID_ID" << setw(8) << "BID" << setw(10) << "PRICE" << setw(8) << "ASK" << setw(8) << "ASK_ID" << setw(6) << endl;

        for(auto& bid : _bidPriceLevel)
        {
            // bid > ask, display ask levels
            while(askIter != _askPriceLevel.end() && bid._price > (*askIter)._price)
            {
                cout << left << setw(8) << " " << setw(8) << " ";
                cout << left << setw(10) << (*askIter)._price << setw(8) << (*askIter)._quantity << setw(8) << (*askIter)._id << endl;
                ++askIter;
            }

            // display bid level
            cout << left << setw(8) << bid._id << setw(8) << bid._quantity << setw(10) << bid._price;

            // crossed level - display crossed ask level
            if(askIter != _askPriceLevel.end() && doubleEq(bid._price, (*askIter)._price))
            {
                cout << left << setw(8) << (*askIter)._quantity << setw(8) << (*askIter)._id << setw(6) << "x" << endl;
                ++askIter;
            }
            else
            {
                cout << setw(30) << " " << endl;
            }
        }

        // display remainder of the ask level
        while(askIter != _askPriceLevel.end())
        {
            cout << left << setw(8) << " " << setw(8) << " ";
            cout << left << setw(10) << (*askIter)._price << setw(8) << (*askIter)._quantity << setw(8) << (*askIter)._id << endl;
            ++askIter;
        }
    }

private:
    bool doubleEq(double a, double b)
    {
        return fabs(a-b) < std::numeric_limits<double>::epsilon();
    }

    template<typename PriceLevelType>
    void addOrder(OrderType& order, PriceLevelType& pl)
    {

        auto result_pair = pl.emplace(order);
        // TODO - check for result of insert - report error if fail
    }

    template<typename PriceLevelType>
    void modifyOrder(OrderType& order, PriceLevelType& pl)
    {
        auto iter = std::find_if(pl.begin(), pl.end(), [&order](const OrderType& eachOrder){
            return order._id == eachOrder._id;
        });

        if(iter != pl.end())
        {
            (*iter) = order;
        }
        // else report error
    }

    template<typename PriceLevelType>
    bool removeOrder(int id, PriceLevelType& pl)
    {
        auto iter = std::find_if(pl.begin(), pl.end(), [&id](const OrderType& eachOrder){
            return id == eachOrder._id;
        });

        if(iter != pl.end())
        {
            pl.erase(iter);
            return true;
        }

        return false;
    }

    template<typename PriceLevelType>
    bool modifyOrder(int id, int quantity, PriceLevelType& pl)
    {
        auto iter = std::find_if(pl.begin(), pl.end(), [&id](const OrderType& eachOrder){
            return id == eachOrder._id;
        });

        if(iter != pl.end())
        {
            (*iter)._quantity = quantity;
            return true;
        }

        return false;
    }

    template<typename PriceLevelType>
    double getPrice(int level, const PriceLevelType& pl)
    {
        int currLevel = 0;
        double lastPrice = 0.0;
        for(const OrderType& order : pl)
        {
            if(!doubleEq(lastPrice, order._price))
            {
                if(!doubleEq(lastPrice, 0.0))
                {
                    ++currLevel;
                }
                lastPrice = order._price;
            }

            if(level == currLevel)
            {
                return order._price;
            }
            // else - keep searching
        }

        return 0.0; // not found
    }

    template<typename PriceLevelType>
    int getSize(int level, const PriceLevelType& pl)
    {
        int currLevel = 0;
        double lastPrice = 0.0;
        int totalSize = 0;
        for(const OrderType& order : pl)
        {
            if(!doubleEq(lastPrice, order._price))
            {
                if(!doubleEq(lastPrice, 0.0))
                {
                    ++currLevel;
                    if(currLevel > level)
                    {
                        return totalSize;
                    }
                }
                lastPrice = order._price;
            }

            if(level == currLevel)
            {
                totalSize += order._quantity;
            }
            // else - keep searching
        }

        return 0; // not found
    }

    BidPiceLevelType _bidPriceLevel;
    AskPiceLevelType _askPriceLevel;
};



#endif /* BOOK_H_ */
