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
    // Release bid price levels
    for (auto& bid : _bids)
        _level_pool.Release(&bid);
    _bids.clear();

    // Release ask price levels
    for (auto& ask : _asks)
        _level_pool.Release(&ask);
    _asks.clear();
}

Level* OrderBook::FindLevel(OrderSide side, uint64_t price) noexcept
{
    Level required(price);

    if (side == OrderSide::BUY)
    {
        auto it = _bids.find(required);
        if (it != _bids.end())
            return it.operator->();
        else
            return nullptr;
    }
    else
    {
        auto it = _asks.find(required);
        if (it != _asks.end())
            return it.operator->();
        else
            return nullptr;
    }
}

Level* OrderBook::AddLevel(Order* order_ptr)
{
    Level* level_ptr = _level_pool.Create(order_ptr->Price);

    if (order_ptr->Side == OrderSide::BUY)
    {
        _bids.insert(*level_ptr);

        // Update best bid price level
        if ((_best_bid == nullptr) || (level_ptr->Price > _best_bid->Price))
            _best_bid = level_ptr;
    }
    else
    {
        _asks.insert(*level_ptr);

        // Update best ask price level
        if ((_best_ask == nullptr) || (level_ptr->Price < _best_ask->Price))
            _best_ask = level_ptr;
    }

    return level_ptr;
}

Level* OrderBook::DeleteLevel(Order* order_ptr, Level* level_ptr)
{
    if (order_ptr->Side == OrderSide::BUY)
    {
        // Update best bid price level
        if (level_ptr == _best_bid)
            _best_bid = _best_bid->parent;

        _bids.erase(Levels::iterator(&_bids, level_ptr));
    }
    else
    {
        // Update best ask price level
        if (level_ptr == _best_ask)
            _best_ask = _best_ask->parent;

        _asks.erase(Levels::iterator(&_asks, level_ptr));
    }

    // Release the price level
    _level_pool.Release(level_ptr);

    // Clear the price level in the given order
    order_ptr->_level = nullptr;

    return nullptr;
}

std::pair<Level*, bool> OrderBook::AddOrder(Order* order_ptr)
{
    // Find the price level for the order
    Level* level_ptr = FindLevel(order_ptr->Side, order_ptr->Price);

    // Create a new price level if no one found
    if (level_ptr == nullptr)
        level_ptr = AddLevel(order_ptr);

    // Update the price level volume
    level_ptr->Volume += order_ptr->Quantity;

    // Link the new order to the orders list of the price level
    level_ptr->Orders.push_back(*order_ptr);

    // Cache the price level in the given order
    order_ptr->_level = level_ptr;

    // Price level was changed. Return top of the book modification flag.
    return std::make_pair(level_ptr, (level_ptr == ((order_ptr->Side == OrderSide::BUY) ? _best_bid : _best_ask)));
}

std::pair<Level*, bool> OrderBook::ReduceOrder(Order* order_ptr, uint64_t quantity)
{
    // Find the price level for the order
    Level* level_ptr = order_ptr->_level;

    // Reduce the order in the price level
    if (level_ptr != nullptr)
    {
        // Update the price level volume
        level_ptr->Volume -= quantity;

        // Unlink the empty order from the orders list of the price level
        if (order_ptr->Quantity == 0)
            level_ptr->Orders.pop_current(*order_ptr);

        // Delete the empty price level
        if (level_ptr->Volume == 0)
            level_ptr = DeleteLevel(order_ptr, level_ptr);

        // Price level was changed. Return top of the book modification flag.
        return std::make_pair(level_ptr, (level_ptr == ((order_ptr->Side == OrderSide::BUY) ? _best_bid : _best_ask)));
    }

    // Price level was not changed
    return std::make_pair(nullptr, false);
}

std::pair<Level*, bool> OrderBook::DeleteOrder(Order* order_ptr)
{
    // Find the price level for the order
    Level* level_ptr = order_ptr->_level;

    // Delete the order from the price level
    if (level_ptr != nullptr)
    {
        // Update the price level volume
        level_ptr->Volume -= order_ptr->Quantity;

        // Unlink the order from the orders list of the price level
        level_ptr->Orders.pop_current(*order_ptr);

        // Delete the empty price level
        if (level_ptr->Volume == 0)
            level_ptr = DeleteLevel(order_ptr, level_ptr);

        // Price level was changed. Return top of the book modification flag.
        return std::make_pair(level_ptr, (level_ptr == ((order_ptr->Side == OrderSide::BUY) ? _best_bid : _best_ask)));
    }

    // Price level was not changed
    return std::make_pair(nullptr, false);
}

} // namespace CppTrader
