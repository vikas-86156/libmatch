#include <iostream>
#include "orderbook.h"

int main()
{

    
    OrderBook orderBook;
    std::cout<<"Adding Buy Order with price 100.0 and quantity 10"<<std::endl;
    orderBook.addOrder({0, Side::BUY, 100.0, 10});
    orderBook.printOrderBook();
    std::cout<<"Adding Sell Order with price 99.0 and quantity 5"<<std::endl;
    orderBook.addOrder({0, Side::SELL, 99.0, 5});
    orderBook.printOrderBook();
    orderBook.addOrder({0, Side::SELL, 99.0, 15});
    orderBook.printOrderBook();
    orderBook.addOrder({0, Side::SELL, 99.0, 15});
    orderBook.printOrderBook();
    std::cout<<"Adding Sell Order with price 101.0 and quantity 5"<<std::endl;
    orderBook.addOrder({0, Side::SELL, 101.0, 50});
    orderBook.printOrderBook();
    std::cout<<"Adding Buy Order with price 98.0 and quantity 5"<<std::endl;
    orderBook.addOrder({0, Side::BUY, 98.0, 5});
    orderBook.printOrderBook();
    std::cout<<"Adding Sell Order with price 100.0 and quantity 10"<<std::endl;
    orderBook.addOrder({0, Side::SELL, 100.0, 10});
    orderBook.printOrderBook();
    std::cout<<"Adding Buy Order with price 101.0 and quantity 5"<<std::endl;
    orderBook.addOrder({0, Side::BUY, 101.0, 5});
    orderBook.printOrderBook();
    double bestBidPrice, bestAskPrice;
    if(orderBook.bestBid(bestBidPrice))
    {
        std::cout<<"Best Bid Price: "<<bestBidPrice<<std::endl;
    }
    if(orderBook.bestAsk(bestAskPrice))
    {
        std::cout<<"Best Ask Price: "<<bestAskPrice<<std::endl;
    }


    return 0;
}