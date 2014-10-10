/*
 * main.cpp
 *
 *  Created on: 6 Oct 2014
 *      Author: Wei Liew (wei@onesixeightsolutions.com)
 *
 *  Copyright Wei Liew 2012 - 2014.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include "Order.h"
#include "Book.h"

template<typename OrderBookType, typename... Args>
void addOrderHelper(OrderBookType& ob, Args... args)
{
    Order tmpOrder(args ...);
    ob.add(tmpOrder);
}

int main(int argc, char* argv[])
{
    typedef Book<Order> OrderBookType;

    OrderBookType ob;
    addOrderHelper(ob, 10001, 'B', 99.1, 100);
    addOrderHelper(ob, 10002, 'B', 99.2, 100);
    addOrderHelper(ob, 10003, 'B', 99.3, 100);
    addOrderHelper(ob, 10004, 'B', 99.3, 100);
    addOrderHelper(ob, 10005, 'B', 99.3, 100);

    addOrderHelper(ob, 10006, 'O', 99.4, 100);
    addOrderHelper(ob, 10007, 'O', 99.5, 100);
    addOrderHelper(ob, 10008, 'O', 99.6, 100);
    addOrderHelper(ob, 10009, 'O', 99.7, 100);
    addOrderHelper(ob, 10010, 'O', 99.7, 100);
    ob.print();

    ob.modify(10001, 200);
    ob.print();

    ob.remove(10010);
    ob.print();

    std::cout << "Level 0 bid: " << ob.getPrice(0, 'B') << std::endl;
    std::cout << "Level 2 ask: " << ob.getPrice(2, 'O') << std::endl;

    std::cout << "Level 0 bid size: " << ob.getSize(0, 'B') << std::endl;
    std::cout << "Level 2 ask size: " << ob.getSize(2, 'O') << std::endl;

    std::cout << "BID:" << std::endl;
    ob.printSide('B');
    std::cout << "ASK:" << std::endl;
    ob.printSide('O');
}

