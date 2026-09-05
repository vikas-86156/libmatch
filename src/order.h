#pragma once
#include <cstdint>


enum class Side : uint8_t {
    BUY,
    SELL
};

struct Order{
    uint64_t orderId;
    Side side;
    double price;
    uint64_t quantity;
    uint64_t timestamp;

    int tick;
    Order* prev=nullptr;
    Order* next=nullptr;

    
};


struct Trade{
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    double price;
    uint64_t quantity;
};



