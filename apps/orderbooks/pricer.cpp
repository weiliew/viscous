/*
 * pricer.cpp
 *
 *  Created on: 21 May 2013
 *      Author: Wei Liew [wei.liew@outlook.com]
 *
 *  Copyright Wei Liew 2012 - 2013.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See http://www.boost.org/LICENSE_1_0.txt)
 * 
 */

#include "Book.h"
#include <vector>
#include <unordered_map>
#include <limits>

bool doubleIsEqual(double a, double b)
{
    //return fabs(a - b) < std::numeric_limits<double>::epsilon();

    // from testing against the expected output, we need to assumed total price is equal correct to 2 decimal point -
    return fabs(a - b) < 0.005;
}

template<typename PriceLevelType>
void calcPriceLevel(const PriceLevelType& pl, int targetSize, char action, const char timestamp [], double& prevPrice)
{
    // not the most efficient manner to check for changes as we are calculating at every book change. A more efficient
    // way would be to calculate only if the top x size has changed.

    bool printNA = true;
    unsigned int totalSize = 0;
    double totalPrice = 0.0;
    for(auto& order : pl)
    {
        // for each order in the price level, accumulate the size till we hit the target size
        if(totalSize + order.size() >= targetSize)
        {
            totalPrice += order.price() * (targetSize - totalSize);
            totalSize = targetSize;

            if(!doubleIsEqual(totalPrice, prevPrice))
            {
                printf("%s %c %.2f\n", timestamp, action, totalPrice);
                prevPrice = totalPrice;
            }
            printNA = false;
            break;
        }
        else
        {
            totalSize += order.size();
            totalPrice += order.price() * order.size();
        }
    }

    if(printNA && !doubleIsEqual(prevPrice, 0.0))
    {
        // printNA
        std::cout << timestamp << " " << action << " " << "NA" << std::endl;
        prevPrice = 0.0;
    }
}

template<typename BookType>
void calcPrice(const BookType& book, int targetSize, const char timestamp [])
{
    static double prevBuyPrice = 0.0;
    static double prevSellPrice = 0.0;

    if(targetSize <= 0)
    {
        return;
    }

    calcPriceLevel(book.getBidPriceLevel(), targetSize, 'S', timestamp, prevSellPrice);
    calcPriceLevel(book.getAskPriceLevel(), targetSize, 'B', timestamp, prevBuyPrice);
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <target size>" << std::endl;
        return -1;
    }

    // use simple stream get with fixed buffer size
    const int buffSize = 1024;
    char lineBuf[buffSize];

    std::vector<const char *> tokens;
    tokens.reserve(6); // max field per message is 6

    typedef Order<std::string> OrderType;
    typedef Book<OrderType>    BookType;

    // using std::string as key - can be optimised to use const char * instead with boost hash func
    typedef std::unordered_map<std::string, OrderType> OrderMapType;

    BookType orderBook;
    OrderMapType orderMap;

    // keep reading in new line until an empty line is found
    while(true)
    {
        std::cin.getline (lineBuf,buffSize);

        if(lineBuf[0] == '\n')
        {
            break;
        }

        // simple c style parsing - parse and push each message field into a token vector
        int count=0;
        int numTokens = 1;
        tokens.clear();
        tokens.push_back((const char *)&lineBuf); // first token
        while(true)
        {
            if(lineBuf[count] == ' ')
            {
                lineBuf[count] = '\0';
                tokens.push_back((const char *)&lineBuf[count+1]);
                ++numTokens;
            }
            else if(lineBuf[count] == '\0')
            {
                // end of line
                break;
            }

            ++count;

            if(count >= buffSize)
            {
                // buffer too small
                break;
            }
        }

        // from specification, we can either check for the action field or just the count of the fields to indicate msg type
        if(numTokens == 6)
        {
            // this is Add
            // we are making assumptions here -
            // 1. Input data format is correct as long as the number of elements matches
            // 2. price is always 2 decimal point
            // 3. size is always a whole number
            // 4. we do not get malformed input (i.e. double spaces)

            // make sure order don't already exist
            const std::string id(tokens[2]);
            OrderMapType::iterator iter = orderMap.find(id);
            if(iter != orderMap.end())
            {
                std::cerr << "Duplicate add order for order id " << tokens[2] << std::endl;
                break;
            }

            // add to the book
            if(*(tokens[3]) == 'B')
            {
                orderBook.addBid(id, rint(atof(tokens[4])*100), atoi(tokens[5]), *(tokens[3]) == 'B' ? OrderType::BID : OrderType::ASK);
            }
            else
            {
                orderBook.addAsk(id, rint(atof(tokens[4])*100), atoi(tokens[5]), *(tokens[3]) == 'B' ? OrderType::BID : OrderType::ASK);
            }

            // add it to our hash map
            orderMap.emplace(id, OrderType(id, rint(atof(tokens[4])*100), atoi(tokens[5]), *(tokens[3]) == 'B' ? OrderType::BID : OrderType::ASK));
        }
        else if(numTokens == 4)
        {
            // this is remove - based on the input format, we will only get the order id and not the price and side, we need to get this info from our map
            std::string id(tokens[2]);
            OrderMapType::iterator iter = orderMap.find(id);
            if(iter == orderMap.end())
            {
                std::cerr << "Id not found for reduce order for order id " << tokens[2] << std::endl;
                break;
            }

            unsigned int changeSize = atoi(tokens[3]);
            OrderType modOrder(iter->second);
            modOrder.setSize(changeSize);
            modOrder.setAction(OrderType::REDUCE);

            orderBook.modifyOrder(modOrder);

            // reduce the size or remove the order from our own map
            if(iter->second.size() == changeSize)
            {
                // remove this from the hash map
                orderMap.erase(iter);
            }
            else
            {
                // reduce the size of the order from the book
                iter->second.setSize(iter->second.size() - changeSize);
            }
        }
        else
        {
            // something is wrong with the input or empty lines signifying an exit
            break;
        }

        // probably want to use a cmd line arg to control printing of the book
        //orderBook.print();

        // here - we run through our calculations for the price based on the target size
        calcPrice(orderBook, atoi(argv[1]), tokens[0]);
    }
}
