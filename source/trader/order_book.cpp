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

std::pair<Level*, bool> OrderBook::AddOrder(Order* order_ptr)
{
    // Find the price level for the order
    Levels::iterator level_it = FindLevel(order_ptr->Side, order_ptr->Price);
    Level* level_ptr = nullptr;

    // Create a new price level if no one found
    if (!level_it)
    {
        level_ptr = _pool.Create(order_ptr->Price);
        if (order_ptr->Side == OrderSide::BUY)
            _bids.insert(*level_ptr);
        else
            _asks.insert(*level_ptr);
    }
    else
        level_ptr = &(*level_it);

    // Update the price level volume
    level_ptr->Volume += order_ptr->Quantity;

    // Link the new order to the orders list of the price level
    level_ptr->Orders.push_back(*order_ptr);

    return std::make_pair(level_ptr, ((order_ptr->Side == OrderSide::BUY) ? ((level_ptr == _bids.highest()) || _bids.empty()) : ((level_ptr == _asks.lowest()) || _asks.empty())));
}

std::pair<Level*, bool> OrderBook::ReduceOrder(Order* order_ptr, uint64_t quantity)
{
    // Find the price level for the order
    Levels::iterator level_it = FindLevel(order_ptr->Side, order_ptr->Price);
    Level* level_ptr = nullptr;

    // Reduce the order in the price level
    if (level_it)
    {
        level_ptr = &(*level_it);

        // Update the price level volume
        level_ptr->Volume -= quantity;

        // Unlink the empty order from the orders list of the price level
        if (order_ptr->Quantity == 0)
            level_ptr->Orders.pop_current(*order_ptr);

        // Delete the empty price level
        if (level_ptr->Volume == 0)
        {
            if (order_ptr->Side == OrderSide::BUY)
                _bids.erase(level_it);
            else
                _asks.erase(level_it);
            _pool.Release(level_ptr);
        }

        return std::make_pair(level_ptr, ((order_ptr->Side == OrderSide::BUY) ? ((level_ptr == _bids.highest()) || _bids.empty()) : ((level_ptr == _asks.lowest()) || _asks.empty())));
    }

    return std::make_pair(nullptr, false);
}

std::pair<Level*, bool> OrderBook::DeleteOrder(Order* order_ptr)
{
    // Find the price level for the order
    Levels::iterator level_it = FindLevel(order_ptr->Side, order_ptr->Price);
    Level* level_ptr = nullptr;

    // Delete the order from the price level
    if (level_it)
    {
        level_ptr = &(*level_it);

        // Update the price level volume
        level_ptr->Volume -= order_ptr->Quantity;

        // Unlink the order from the orders list of the price level
        level_ptr->Orders.pop_current(*order_ptr);

        // Delete the empty price level
        if (level_ptr->Volume == 0)
        {
            if (order_ptr->Side == OrderSide::BUY)
                _bids.erase(level_it);
            else
                _asks.erase(level_it);
            _pool.Release(level_ptr);
        }

        return std::make_pair(level_ptr, ((order_ptr->Side == OrderSide::BUY) ? ((level_ptr == _bids.highest()) || _bids.empty()) : ((level_ptr == _asks.lowest()) || _asks.empty())));
    }

    return std::make_pair(nullptr, false);
}

} // namespace CppTrader
