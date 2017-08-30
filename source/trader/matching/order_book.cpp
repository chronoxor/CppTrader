/*!
    \file order_book.cpp
    \brief Order book implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#include "trader/matching/order_book.h"

namespace CppTrader {
namespace Matching {

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

LevelNode* OrderBook::AddLevel(OrderNode* order_ptr)
{
    LevelNode* level_ptr = nullptr;

    if (order_ptr->IsBuy())
    {
        // Create a new price level
        level_ptr = _level_pool.Create(LevelType::BID, order_ptr->Price);

        // Insert the price level into the bid collection
        _bids.insert(*level_ptr);

        // Update best bid price level
        if ((_best_bid == nullptr) || (level_ptr->Price > _best_bid->Price))
            _best_bid = level_ptr;
    }
    else
    {
        // Create a new price level
        level_ptr = _level_pool.Create(LevelType::ASK, order_ptr->Price);

        // Insert the price level into the ask collection
        _asks.insert(*level_ptr);

        // Update best ask price level
        if ((_best_ask == nullptr) || (level_ptr->Price < _best_ask->Price))
            _best_ask = level_ptr;
    }

    return level_ptr;
}

LevelNode* OrderBook::DeleteLevel(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    if (order_ptr->IsBuy())
    {
        // Update best bid price level
        if (level_ptr == _best_bid)
            _best_bid = (_best_bid->left != nullptr) ? _best_bid->left : _best_bid->parent;

        // Erase the price level from the bid collection
        _bids.erase(Levels::iterator(&_bids, level_ptr));
    }
    else
    {
        // Update best ask price level
        if (level_ptr == _best_ask)
            _best_ask = (_best_ask->right != nullptr) ? _best_ask->right : _best_ask->parent;

        // Erase the price level from the ask collection
        _asks.erase(Levels::iterator(&_asks, level_ptr));
    }

    // Release the price level
    _level_pool.Release(level_ptr);

    return nullptr;
}

LevelUpdate OrderBook::AddLimitOrder(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->IsBuy() ? (LevelNode*)GetBid(order_ptr->Price) : (LevelNode*)GetAsk(order_ptr->Price);

    // Create a new price level if no one found
    UpdateType update = UpdateType::UPDATE;
    if (level_ptr == nullptr)
    {
        level_ptr = AddLevel(order_ptr);
        update = UpdateType::ADD;
    }

    // Update the price level volume
    level_ptr->Volume += order_ptr->Quantity;

    // Link the new order to the orders list of the price level
    level_ptr->OrderList.push_back(*order_ptr);
    ++level_ptr->Orders;

    // Cache the price level in the given order
    order_ptr->Level = level_ptr;

    // Price level was changed. Return top of the book modification flag.
    return LevelUpdate(update, *order_ptr->Level, (order_ptr->Level == (order_ptr->IsBuy() ? _best_bid : _best_ask)));
}

LevelUpdate OrderBook::ReduceLimitOrder(OrderNode* order_ptr, uint64_t quantity)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->Volume -= quantity;

    // Unlink the empty order from the orders list of the price level
    if (order_ptr->Quantity == 0)
    {
        level_ptr->OrderList.pop_current(*order_ptr);
        --level_ptr->Orders;
    }

    Level level(*level_ptr);

    // Delete the empty price level
    UpdateType update = UpdateType::UPDATE;
    if (level_ptr->Volume == 0)
    {
        // Clear the price level cache in the given order
        order_ptr->Level = DeleteLevel(order_ptr);
        update = UpdateType::DELETE;
    }

    // Price level was changed. Return top of the book modification flag.
    return LevelUpdate(update, level, (order_ptr->Level == (order_ptr->IsBuy() ? _best_bid : _best_ask)));
}

LevelUpdate OrderBook::DeleteLimitOrder(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->Volume -= order_ptr->Quantity;

    // Unlink the empty order from the orders list of the price level
    level_ptr->OrderList.pop_current(*order_ptr);
    --level_ptr->Orders;

    Level level(*level_ptr);

    // Delete the empty price level
    UpdateType update = UpdateType::UPDATE;
    if (level_ptr->Volume == 0)
    {
        // Clear the price level cache in the given order
        order_ptr->Level = DeleteLevel(order_ptr);
        update = UpdateType::DELETE;
    }

    // Price level was changed. Return top of the book modification flag.
    return LevelUpdate(update, level, (order_ptr->Level == (order_ptr->IsBuy() ? _best_bid : _best_ask)));
}

} // namespace Matching
} // namespace CppTrader
