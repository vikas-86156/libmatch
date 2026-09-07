#pragma once
#include <cstdint>
#include <map>
#include <deque>
#include "order.h"
#include <unordered_map>
#include <vector>
#include "orderPool.h"

struct pricelevel{
    Order* head=nullptr;
    Order* tail=nullptr;
};

struct addOrderResult{
    bool accepted;
    std::vector<Trade> trades;
};

class OrderBook
{
    public :


    addOrderResult addOrder(Order order );
    bool cancelOrder(uint64_t orderId);
    
    explicit OrderBook(size_t poolCapacity = 100000);
    bool bestBid(double &priceout) const;
    bool bestAsk(double &priceout) const;

    void printOrderBook() const;
    ~OrderBook();

    private:
    static constexpr double maxPrice=110.00;
    static constexpr double minPrice=90.00;
    static constexpr double tickSize=0.05;
    static constexpr int numTicks=(maxPrice-minPrice)/tickSize+1;
    
    std::vector<pricelevel> bids=std::vector<pricelevel>(numTicks);
    std::vector<pricelevel> asks=std::vector<pricelevel>(numTicks);

    std::unordered_map<uint64_t,Order*> orderIndex;

    int bestBidTick=-1;
    int bestAskTick=numTicks;
    OrderPool orderPool;

    static double tickToPrice(int tick);
    static bool priceRange(double price);
    static int priceToTick(double price);


    void unlink(Order* order,std::vector<pricelevel>  &ladder);
    void appendlink(Order* order,std::vector<pricelevel>  &ladder);

    int firstOccupiedBidFrom(int tick) const;  // scans downward, -1 if none
    int firstOccupiedAskFrom(int tick) const;  // scans upward, numTicks if none

    // std::map<double,std::deque<Order>,std::greater<double>> bids;
    // std::map<double,std::deque<Order>,std::less<double>> asks;




    uint64_t timestampCounter=0;
    uint64_t orderIdCounter=0;

};