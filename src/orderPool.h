#pragma once
#include  "order.h"
#include<cstddef>

class OrderPool
{
    public:

    explicit OrderPool(size_t poolSize);
    ~OrderPool();

    OrderPool(const OrderPool&)=delete;
    OrderPool & operator =(const OrderPool&)=delete;


    Order* allocate(const Order &order);
    void deallocate(Order* order);

    private:
    static constexpr size_t CACHE_LINE=64;
    static constexpr size_t computeBlockSize();
    // {
    //     return sizeof(Order) * poolSize;
    // }
    struct FreeNode{
        FreeNode* next;
    };
    FreeNode* freeList;

    size_t capacity;
    size_t blockSize;
    std:: byte* buffer;
    FreeNode* freeListHead;

};