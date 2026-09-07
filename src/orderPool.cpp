#include "orderPool.h"
#include <new>
#include <cassert>
#include<cstdint>




OrderPool::OrderPool(size_t poolSize)
{
    capacity = poolSize;
    blockSize = computeBlockSize();

    size_t totalBytes = blockSize * capacity;

    buffer=static_cast<std::byte*>(operator new(totalBytes,std::align_val_t(CACHE_LINE)));
    assert(reinterpret_cast<uintptr_t>(buffer) % CACHE_LINE == 0);

    for(size_t i=0;i<capacity;i++)
    {
        std::byte* blockptr=buffer+i*blockSize;
        FreeNode* node = reinterpret_cast<FreeNode*>(blockptr);
        node->next = (i + 1 < capacity)
            ? reinterpret_cast<FreeNode*>(buffer + (i + 1) * blockSize)
            : nullptr;
    }

    freeListHead = reinterpret_cast<FreeNode*>(buffer);
    
}

OrderPool::~OrderPool()
{
    ::operator delete(buffer,std::align_val_t(CACHE_LINE));
}

Order* OrderPool::allocate(const Order &order)
{
    if(freeListHead==nullptr)
    {
        throw std::bad_alloc();
    }

    FreeNode* node = freeListHead;
    freeListHead = freeListHead->next;

    Order* orderPtr = reinterpret_cast<Order*>(node);
    new (orderPtr) Order(order);

    return orderPtr;
}

void OrderPool::deallocate(Order* order)
{
    if(order==nullptr)
    {
        return;
    }

    order->~Order();

    FreeNode* node = reinterpret_cast<FreeNode*>(order);
    node->next = freeListHead;
    freeListHead = node;
}
constexpr size_t OrderPool::computeBlockSize()
{
    size_t raw = sizeof(Order) > sizeof(FreeNode) ? sizeof(Order) : sizeof(FreeNode);
    return ((raw + CACHE_LINE - 1) / CACHE_LINE) * CACHE_LINE;
}