/*!
    \file order_book.cpp
    \brief Order book implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#include "trader/order_book.h"

namespace CppTrader {

OrderBook::~OrderBook()
{
    for (auto& bid : _bids)
        _pool.Release(&bid);
    _bids.clear();

    for (auto& ask : _asks)
        _pool.Release(&ask);
    _asks.clear();
}

OrderBook::Levels::iterator OrderBook::FindLevel(OrderSide side, uint64_t price) noexcept
{
    Level required(price);

    if (side == OrderSide::BUY)
    {
        auto it = _bids.find(required);
        if (it != _bids.end())
            return it;
        else
            return _bids.end();
    }
    else
    {
        auto it = _asks.find(required);
        if (it != _asks.end())
            return it;
        else
            return _asks.end();
    }
}

void OrderBook::AddOrder(Order* order)
{
    // Find the price level for the order
    Levels::iterator it_level = FindLevel(order->Side, order->Price);
    Level* level = nullptr;

    // Create a new price level if no one found
    if (!it_level)
    {
        level = _pool.Create(order->Price);
        if (order->Side == OrderSide::BUY)
            _bids.insert(*level);
        else
            _asks.insert(*level);
    }
    else
        level = &(*it_level);

    // Update the price level volume
    level->Volume += order->Quantity;

    // Link the new order to the orders list of the price level
    level->Orders.push_back(*order);
}

void OrderBook::ReduceOrder(Order* order, uint64_t quantity)
{
    // Find the price level for the order
    Levels::iterator it_level = FindLevel(order->Side, order->Price);
    Level* level = nullptr;

    // Reduce the order in the price level
    if (it_level)
    {
        level = &(*it_level);

        // Update the price level volume
        level->Volume -= quantity;

        // Unlink the empty order from the orders list of the price level
        if (order->Quantity == 0)
            level->Orders.pop_current(*order);

        // Delete the empty price level
        if (level->Volume == 0)
        {
            if (order->Side == OrderSide::BUY)
                _bids.erase(it_level);
            else
                _asks.erase(it_level);
            _pool.Release(level);
        }
    }
}

void OrderBook::DeleteOrder(Order* order)
{
    // Find the price level for the order
    Levels::iterator it_level = FindLevel(order->Side, order->Price);
    Level* level = nullptr;

    // Delete the order from the price level
    if (it_level)
    {
        level = &(*it_level);

        // Update the price level volume
        level->Volume -= order->Quantity;

        // Unlink the order from the orders list of the price level
        level->Orders.pop_current(*order);

        // Delete the empty price level
        if (level->Volume == 0)
        {
            if (order->Side == OrderSide::BUY)
                _bids.erase(it_level);
            else
                _asks.erase(it_level);
            _pool.Release(level);
        }
    }
}

} // namespace CppTrader
