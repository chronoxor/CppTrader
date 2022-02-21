/*!
    \file order_book.cpp
    \brief Order book implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#include "trader/matching/market_manager.h"
#include "trader/matching/order_book.h"

namespace CppTrader {
namespace Matching {

OrderBook::OrderBook(MarketManager& manager, const Symbol& symbol)
    : _manager(manager),
      _symbol(symbol),
      _best_bid(nullptr),
      _best_ask(nullptr),
      _best_buy_stop(nullptr),
      _best_sell_stop(nullptr),
      _best_trailing_buy_stop(nullptr),
      _best_trailing_sell_stop(nullptr),
      _last_bid_price(0),
      _last_ask_price(std::numeric_limits<uint64_t>::max()),
      _matching_bid_price(0),
      _matching_ask_price(std::numeric_limits<uint64_t>::max()),
      _trailing_bid_price(0),
      _trailing_ask_price(std::numeric_limits<uint64_t>::max())
{
}

OrderBook::~OrderBook()
{
    // Release bid price levels
    for (auto& bid : _bids)
        _manager._level_pool.Release(&bid);
    _bids.clear();

    // Release ask price levels
    for (auto& ask : _asks)
        _manager._level_pool.Release(&ask);
    _asks.clear();

    // Release buy stop orders levels
    for (auto& buy_stop : _buy_stop)
        _manager._level_pool.Release(&buy_stop);
    _buy_stop.clear();

    // Release sell stop orders levels
    for (auto& sell_stop : _sell_stop)
        _manager._level_pool.Release(&sell_stop);
    _sell_stop.clear();

    // Release trailing buy stop orders levels
    for (auto& trailing_buy_stop : _trailing_buy_stop)
        _manager._level_pool.Release(&trailing_buy_stop);
    _trailing_buy_stop.clear();

    // Release trailing sell stop orders levels
    for (auto& trailing_sell_stop : _trailing_sell_stop)
        _manager._level_pool.Release(&trailing_sell_stop);
    _trailing_sell_stop.clear();
}

LevelNode* OrderBook::AddLevel(OrderNode* order_ptr)
{
    LevelNode* level_ptr = nullptr;

    if (order_ptr->IsBuy())
    {
        // Create a new price level
        level_ptr = _manager._level_pool.Create(LevelType::BID, order_ptr->Price);

        // Insert the price level into the bid collection
        _bids.insert(*level_ptr);

        // Update the best bid price level
        if ((_best_bid == nullptr) || (level_ptr->Price > _best_bid->Price))
            _best_bid = level_ptr;
    }
    else
    {
        // Create a new price level
        level_ptr = _manager._level_pool.Create(LevelType::ASK, order_ptr->Price);

        // Insert the price level into the ask collection
        _asks.insert(*level_ptr);

        // Update the best ask price level
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
        // Update the best bid price level
        if (level_ptr == _best_bid)
            _best_bid = (_best_bid->left != nullptr) ? _best_bid->left : ((_best_bid->parent != nullptr) ? _best_bid->parent : _best_bid->right);

        // Erase the price level from the bid collection
        _bids.erase(Levels::iterator(&_bids, level_ptr));
    }
    else
    {
        // Update the best ask price level
        if (level_ptr == _best_ask)
            _best_ask = (_best_ask->right != nullptr) ? _best_ask->right : ((_best_ask->parent != nullptr) ? _best_ask->parent : _best_ask->left);

        // Erase the price level from the ask collection
        _asks.erase(Levels::iterator(&_asks, level_ptr));
    }

    // Release the price level
    _manager._level_pool.Release(level_ptr);

    return nullptr;
}

LevelUpdate OrderBook::AddOrder(OrderNode* order_ptr)
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
    level_ptr->TotalVolume += order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume += order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume += order_ptr->VisibleQuantity();

    // Link the new order to the orders list of the price level
    level_ptr->OrderList.push_back(*order_ptr);
    ++level_ptr->Orders;

    // Cache the price level in the given order
    order_ptr->Level = level_ptr;

    // Price level was changed. Return top of the book modification flag.
    return LevelUpdate(update, *order_ptr->Level, (order_ptr->Level == (order_ptr->IsBuy() ? _best_bid : _best_ask)));
}

LevelUpdate OrderBook::ReduceOrder(OrderNode* order_ptr, uint64_t quantity, uint64_t hidden, uint64_t visible)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->TotalVolume -= quantity;
    level_ptr->HiddenVolume -= hidden;
    level_ptr->VisibleVolume -= visible;

    // Unlink the empty order from the orders list of the price level
    if (order_ptr->LeavesQuantity == 0)
    {
        level_ptr->OrderList.pop_current(*order_ptr);
        --level_ptr->Orders;
    }

    Level level(*level_ptr);

    // Delete the empty price level
    UpdateType update = UpdateType::UPDATE;
    if (level_ptr->TotalVolume == 0)
    {
        // Clear the price level cache in the given order
        order_ptr->Level = DeleteLevel(order_ptr);
        update = UpdateType::DELETE;
    }

    // Price level was changed. Return top of the book modification flag.
    return LevelUpdate(update, level, ((order_ptr->Level == nullptr) || (order_ptr->Level == (order_ptr->IsBuy() ? _best_bid : _best_ask))));
}

LevelUpdate OrderBook::DeleteOrder(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->TotalVolume -= order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume -= order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume -= order_ptr->VisibleQuantity();

    // Unlink the empty order from the orders list of the price level
    level_ptr->OrderList.pop_current(*order_ptr);
    --level_ptr->Orders;

    Level level(*level_ptr);

    // Delete the empty price level
    UpdateType update = UpdateType::UPDATE;
    if (level_ptr->TotalVolume == 0)
    {
        // Clear the price level cache in the given order
        order_ptr->Level = DeleteLevel(order_ptr);
        update = UpdateType::DELETE;
    }

    // Price level was changed. Return top of the book modification flag.
    return LevelUpdate(update, level, ((order_ptr->Level == nullptr) || (order_ptr->Level == (order_ptr->IsBuy() ? _best_bid : _best_ask))));
}

LevelNode* OrderBook::AddStopLevel(OrderNode* order_ptr)
{
    LevelNode* level_ptr = nullptr;

    if (order_ptr->IsBuy())
    {
        // Create a new price level
        level_ptr = _manager._level_pool.Create(LevelType::ASK, order_ptr->StopPrice);

        // Insert the price level into the buy stop orders collection
        _buy_stop.insert(*level_ptr);

        // Update the best buy stop order price level
        if ((_best_buy_stop == nullptr) || (level_ptr->Price < _best_buy_stop->Price))
            _best_buy_stop = level_ptr;
    }
    else
    {
        // Create a new price level
        level_ptr = _manager._level_pool.Create(LevelType::BID, order_ptr->StopPrice);

        // Insert the price level into the sell stop orders collection
        _sell_stop.insert(*level_ptr);

        // Update the best sell stop order price level
        if ((_best_sell_stop == nullptr) || (level_ptr->Price > _best_sell_stop->Price))
            _best_sell_stop = level_ptr;
    }

    return level_ptr;
}

LevelNode* OrderBook::DeleteStopLevel(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    if (order_ptr->IsBuy())
    {
        // Update the best buy stop order price level
        if (level_ptr == _best_buy_stop)
            _best_buy_stop = (_best_buy_stop->right != nullptr) ? _best_buy_stop->right : _best_buy_stop->parent;

        // Erase the price level from the buy stop orders collection
        _buy_stop.erase(Levels::iterator(&_buy_stop, level_ptr));
    }
    else
    {
        // Update the best sell stop order price level
        if (level_ptr == _best_sell_stop)
            _best_sell_stop = (_best_sell_stop->left != nullptr) ? _best_sell_stop->left : _best_sell_stop->parent;

        // Erase the price level from the sell stop orders collection
        _sell_stop.erase(Levels::iterator(&_sell_stop, level_ptr));
    }

    // Release the price level
    _manager._level_pool.Release(level_ptr);

    return nullptr;
}

void OrderBook::AddStopOrder(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->IsBuy() ? (LevelNode*)GetBuyStopLevel(order_ptr->StopPrice) : (LevelNode*)GetSellStopLevel(order_ptr->StopPrice);

    // Create a new price level if no one found
    if (level_ptr == nullptr)
        level_ptr = AddStopLevel(order_ptr);

    // Update the price level volume
    level_ptr->TotalVolume += order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume += order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume += order_ptr->VisibleQuantity();

    // Link the new order to the orders list of the price level
    level_ptr->OrderList.push_back(*order_ptr);
    ++level_ptr->Orders;

    // Cache the price level in the given order
    order_ptr->Level = level_ptr;
}

void OrderBook::ReduceStopOrder(OrderNode* order_ptr, uint64_t quantity, uint64_t hidden, uint64_t visible)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->TotalVolume -= quantity;
    level_ptr->HiddenVolume -= hidden;
    level_ptr->VisibleVolume -= visible;

    // Unlink the empty order from the orders list of the price level
    if (order_ptr->LeavesQuantity == 0)
    {
        level_ptr->OrderList.pop_current(*order_ptr);
        --level_ptr->Orders;
    }

    // Delete the empty price level
    if (level_ptr->TotalVolume == 0)
    {
        // Clear the price level cache in the given order
        order_ptr->Level = DeleteStopLevel(order_ptr);
    }
}

void OrderBook::DeleteStopOrder(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->TotalVolume -= order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume -= order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume -= order_ptr->VisibleQuantity();

    // Unlink the empty order from the orders list of the price level
    level_ptr->OrderList.pop_current(*order_ptr);
    --level_ptr->Orders;

    // Delete the empty price level
    if (level_ptr->TotalVolume == 0)
    {
        // Clear the price level cache in the given order
        order_ptr->Level = DeleteStopLevel(order_ptr);
    }
}

LevelNode* OrderBook::AddTrailingStopLevel(OrderNode* order_ptr)
{
    LevelNode* level_ptr = nullptr;

    if (order_ptr->IsBuy())
    {
        // Create a new price level
        level_ptr = _manager._level_pool.Create(LevelType::ASK, order_ptr->StopPrice);

        // Insert the price level into the trailing buy stop orders collection
        _trailing_buy_stop.insert(*level_ptr);

        // Update the best trailing buy stop order price level
        if ((_best_trailing_buy_stop == nullptr) || (level_ptr->Price < _best_trailing_buy_stop->Price))
            _best_trailing_buy_stop = level_ptr;
    }
    else
    {
        // Create a new price level
        level_ptr = _manager._level_pool.Create(LevelType::BID, order_ptr->StopPrice);

        // Insert the price level into the trailing sell stop orders collection
        _trailing_sell_stop.insert(*level_ptr);

        // Update the best trailing sell stop order price level
        if ((_best_trailing_sell_stop == nullptr) || (level_ptr->Price > _best_trailing_sell_stop->Price))
            _best_trailing_sell_stop = level_ptr;
    }

    return level_ptr;
}

LevelNode* OrderBook::DeleteTrailingStopLevel(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    if (order_ptr->IsBuy())
    {
        // Update the best trailing buy stop order price level
        if (level_ptr == _best_trailing_buy_stop)
            _best_trailing_buy_stop = (_best_trailing_buy_stop->right != nullptr) ? _best_trailing_buy_stop->right : _best_trailing_buy_stop->parent;

        // Erase the price level from the trailing buy stop orders collection
        _trailing_buy_stop.erase(Levels::iterator(&_trailing_buy_stop, level_ptr));
    }
    else
    {
        // Update the best trailing sell stop order price level
        if (level_ptr == _best_trailing_sell_stop)
            _best_trailing_sell_stop = (_best_trailing_sell_stop->left != nullptr) ? _best_trailing_sell_stop->left : _best_trailing_sell_stop->parent;

        // Erase the price level from the trailing sell stop orders collection
        _trailing_sell_stop.erase(Levels::iterator(&_trailing_sell_stop, level_ptr));
    }

    // Release the price level
    _manager._level_pool.Release(level_ptr);

    return nullptr;
}

void OrderBook::AddTrailingStopOrder(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->IsBuy() ? (LevelNode*)GetTrailingBuyStopLevel(order_ptr->StopPrice) : (LevelNode*)GetTrailingSellStopLevel(order_ptr->StopPrice);

    // Create a new price level if no one found
    if (level_ptr == nullptr)
        level_ptr = AddTrailingStopLevel(order_ptr);

    // Update the price level volume
    level_ptr->TotalVolume += order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume += order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume += order_ptr->VisibleQuantity();

    // Link the new order to the orders list of the price level
    level_ptr->OrderList.push_back(*order_ptr);
    ++level_ptr->Orders;

    // Cache the price level in the given order
    order_ptr->Level = level_ptr;
}

void OrderBook::ReduceTrailingStopOrder(OrderNode* order_ptr, uint64_t quantity, uint64_t hidden, uint64_t visible)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->TotalVolume -= quantity;
    level_ptr->HiddenVolume -= hidden;
    level_ptr->VisibleVolume -= visible;

    // Unlink the empty order from the orders list of the price level
    if (order_ptr->LeavesQuantity == 0)
    {
        level_ptr->OrderList.pop_current(*order_ptr);
        --level_ptr->Orders;
    }

    // Delete the empty price level
    if (level_ptr->TotalVolume == 0)
    {
        // Clear the price level cache in the given order
        order_ptr->Level = DeleteTrailingStopLevel(order_ptr);
    }
}

void OrderBook::DeleteTrailingStopOrder(OrderNode* order_ptr)
{
    // Find the price level for the order
    LevelNode* level_ptr = order_ptr->Level;

    // Update the price level volume
    level_ptr->TotalVolume -= order_ptr->LeavesQuantity;
    level_ptr->HiddenVolume -= order_ptr->HiddenQuantity();
    level_ptr->VisibleVolume -= order_ptr->VisibleQuantity();

    // Unlink the empty order from the orders list of the price level
    level_ptr->OrderList.pop_current(*order_ptr);
    --level_ptr->Orders;

    // Delete the empty price level
    if (level_ptr->TotalVolume == 0)
    {
        // Clear the price level cache in the given order
        order_ptr->Level = DeleteTrailingStopLevel(order_ptr);
    }
}

uint64_t OrderBook::CalculateTrailingStopPrice(const Order& order) const noexcept
{
    // Get the current market price
    uint64_t market_price = order.IsBuy() ? GetMarketTrailingStopPriceAsk() : GetMarketTrailingStopPriceBid();
    int64_t trailing_distance = order.TrailingDistance;
    int64_t trailing_step = order.TrailingStep;

    // Convert percentage trailing values into absolute ones
    if (trailing_distance < 0)
    {
        trailing_distance = (int64_t)((-trailing_distance * market_price) / 10000);
        trailing_step = (int64_t)((-trailing_step * market_price) / 10000);
    }

    uint64_t old_price = order.StopPrice;

    if (order.IsBuy())
    {
        // Calculate a new stop price
        uint64_t new_price = (market_price < (std::numeric_limits<uint64_t>::max() - trailing_distance)) ? (market_price + trailing_distance) : std::numeric_limits<uint64_t>::max();

        // If the new price is better and we get through the trailing step
        if (new_price < old_price)
            if ((old_price - new_price) >= (uint64_t)trailing_step)
                return new_price;
    }
    else
    {
        // Calculate a new stop price
        uint64_t new_price = (market_price > (uint64_t)trailing_distance) ? (market_price - trailing_distance) : 0;

        // If the new price is better and we get through the trailing step
        if (new_price > old_price)
            if ((new_price - old_price) >= (uint64_t)trailing_step)
                return new_price;
    }

    return old_price;
}

} // namespace Matching
} // namespace CppTrader
