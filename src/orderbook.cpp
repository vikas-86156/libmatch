#include "orderbook.h"
#include <iostream>
#include <cmath>

addOrderResult OrderBook::addOrder(Order order)
{
    addOrderResult result;
    result.accepted=false;
    if(!priceRange(order.price))
    {
        std::cout<<"Price out of range"<<std::endl;
        return result;
    }



    order.tick=priceToTick(order.price);
    order.orderId=orderIdCounter++;
    result.accepted=true;

    
    order.timestamp=timestampCounter++;

    if(order.side ==Side::BUY)
    {
        while(order.quantity>0 && bestAskTick<numTicks && order.tick>=bestAskTick)
        {

            while(order.quantity>0 && asks[bestAskTick].head!=nullptr)
            {
                if(order.quantity>=asks[bestAskTick].head->quantity)
                {
                    result.trades.push_back({order.orderId,asks[bestAskTick].head->orderId,asks[bestAskTick].head->price,asks[bestAskTick].head->quantity});

                    order.quantity-=asks[bestAskTick].head->quantity;
                    unlink(asks[bestAskTick].head,asks);
                    // result.trades.push_back({order.orderId,asks[bestAskTick].head->orderId,asks[bestAskTick].head->price,asks[bestAskTick].head->quantity});

                }
                else
                {
                    result.trades.push_back({order.orderId,asks[bestAskTick].head->orderId,asks[bestAskTick].head->price,order.quantity});

                    asks[bestAskTick].head->quantity-=order.quantity;
                    order.quantity=0;
                    // result.trades.push_back({order.orderId,asks[bestAskTick].head->orderId,asks[bestAskTick].head->price,order.quantity});
                }
            }
            if(asks[bestAskTick].head==nullptr)
            {
                bestAskTick=firstOccupiedAskFrom(bestAskTick);
            }
            // bestAskTick=firstOccupiedAskFrom(bestAskTick+1);
        }
        if(order.quantity>0)
        {
            Order* newOrder=new Order(order);
            appendlink(newOrder,bids);
            orderIndex[order.orderId]=newOrder;
            if(bestBidTick<order.tick)
            {
                bestBidTick=order.tick;
            }
        }
        // return result;
        
    }
    else if(order.side ==Side::SELL)
    {
        
        while(order.quantity>0 && bestBidTick>=0 && order.tick<=bestBidTick)
        {
            while(order.quantity>0 && bids[bestBidTick].head!=nullptr)
            {
                if(order.quantity>=bids[bestBidTick].head->quantity)
                {
                    result.trades.push_back({bids[bestBidTick].head->orderId,order.orderId,bids[bestBidTick].head->price,bids[bestBidTick].head->quantity});

                    order.quantity-=bids[bestBidTick].head->quantity;
                    // std::cout<<"Unlinking order with id: "<<bids[bestBidTick].head->orderId<<std::endl;
                    unlink(bids[bestBidTick].head,bids);
                    // std::cout<<"Unlinked order with id: "<<bids[bestBidTick].head->orderId<<std::endl;
                    // result.trades.push_back({bids[bestBidTick].head->orderId,order.orderId,bids[bestBidTick].head->price,bids[bestBidTick].head->quantity});
                }
                else
                {
                    result.trades.push_back({bids[bestBidTick].head->orderId,order.orderId,bids[bestBidTick].head->price,order.quantity});

                    bids[bestBidTick].head->quantity-=order.quantity;
                    order.quantity=0;
                    // result.trades.push_back({bids[bestBidTick].head->orderId,order.orderId,bids[bestBidTick].head->price,order.quantity});
                }
            }
            if(bids[bestBidTick].head==nullptr)
            {
                bestBidTick=firstOccupiedBidFrom(bestBidTick);
            }
            // bestBidTick=firstOccupiedBidFrom(bestBidTick-1);
        }
        std::cout<<"done with trades"<<std::endl;
        if(order.quantity>0)
        {
            Order* newOrder=new Order(order);
            appendlink(newOrder,asks);
            orderIndex[order.orderId]=newOrder;
            if(bestAskTick>order.tick)
            {
                bestAskTick=order.tick;
            }
        }
        
        
    }
    return result;

    
}

bool OrderBook:: cancelOrder(uint64_t orderId)
{
    if(orderIndex.find(orderId)==orderIndex.end())
    {
        return false;
    }
    Order* order=orderIndex[orderId];

    if(order->side==Side::BUY)
    {
        unlink(order,bids);
        if(order->tick==bestBidTick && bids[bestBidTick].head==nullptr)
        {
            bestBidTick=firstOccupiedBidFrom(bestBidTick-1);
        }
    }
    else if(order->side==Side::SELL)
    {
        unlink(order,asks);
        if(order->tick==bestAskTick && asks[bestAskTick].head==nullptr)
        {
            bestAskTick=firstOccupiedAskFrom(bestAskTick+1);
        }
    }

    delete order;
    orderIndex.erase(orderId);
    return true;

}


 bool OrderBook :: bestBid(double &priceOut) const{
    if(bestBidTick>=0)
    {
        priceOut=tickToPrice(bestBidTick);
        return true;
    }
    return false;
 }

 bool OrderBook ::bestAsk(double &priceOut) const{
    if(bestAskTick<numTicks)
    {
        priceOut=tickToPrice(bestAskTick);
        return true;
    }
    return false;
 }


 void OrderBook::printOrderBook() const{
    std::cout<<"Order Book:"<<std::endl;
    std::cout<<"Bids:"<<std::endl;
    // std::cout<<"Best Bid Tick: "<<bestBidTick<<std::endl;
    for(int tick=bestBidTick;tick>=0;tick--)
    {
        // std::cout<<bids[tick].head<<std::endl;
        // if(bids[tick].head==nullptr)
        // {
        //     std::cout<<"nullptr";
        // }
        // else
        // {
        //     std::cout<<"not null";
        // }

        if(bids[tick].head!=nullptr)
        {
            // std::cout<<"safe";
            std::cout<<"Price: "<<tickToPrice(tick)<<", Quantity: ";
            Order* order=bids[tick].head;
            while(order!=nullptr)
            {
                std::cout<<order->quantity<<" ";
                order=order->next;
            }
            std::cout<<std::endl;

        }
        // else
        // std::cout<<"nullotr";
    }
    std::cout<<"Asks:"<<std::endl;
    for(int tick=bestAskTick;tick<numTicks;tick++)
    {
        if(asks[tick].head!=nullptr)
        {
            std::cout<<"Price: "<<tickToPrice(tick)<<", Quantity: ";
            Order* order=asks[tick].head;
            while(order!=nullptr)
            {
                std::cout<<order->quantity<<" ";
                order=order->next;
            }
            std::cout<<std::endl;
        }
    }
 }

double OrderBook::tickToPrice(int tick)
{
    return minPrice+tick*tickSize;
}

int OrderBook::priceToTick(double price)
{
    return static_cast<int>(std::round((price-minPrice)/tickSize));
}

bool OrderBook::priceRange(double price)
{
    return price>=minPrice && price<=maxPrice;
}

OrderBook::~OrderBook()
{
    
}

void OrderBook::unlink(Order* order,std::vector<pricelevel>  &ladder)
{
    if(order->prev!=nullptr)
    {
        order->prev->next=order->next;
    }
    else
    {
        ladder[order->tick].head=order->next;
        // std::cout<<"Unlinking order with id: "<<order->orderId<<std::endl;
    }
    if(order->next!=nullptr)
    {
        
        order->next->prev=order->prev;
    }
    else
    {
        ladder[order->tick].tail=order->prev;
        // std::cout<<"Unlinking order with id: "<<order->orderId<<std::endl;
    }
    // std::cout<<"Unlinking order with id: "<<order->orderId<<std::endl;
    order->prev=nullptr;
    order->next=nullptr;
    // std::cout<<"Unlinked order with id: "<<order->orderId<<std::endl;


}

void OrderBook::appendlink(Order* order,std::vector<pricelevel>  &ladder)
{
    if(ladder[order->tick].tail==nullptr)
    {
        ladder[order->tick].head=order;
        ladder[order->tick].tail=order;
    }
    else
    {
        ladder[order->tick].tail->next=order;
        order->prev=ladder[order->tick].tail;
        ladder[order->tick].tail=order;
    }
}


int OrderBook::firstOccupiedBidFrom(int tick) const
{
    while(tick>=0 && bids[tick].head==nullptr)
    {
        tick--;
    }
    return tick;
}

int OrderBook::firstOccupiedAskFrom(int tick) const
{
    while(tick<numTicks && asks[tick].head==nullptr)
    {
        tick++;
    }
    return tick;
}
