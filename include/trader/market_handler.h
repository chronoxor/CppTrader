/*!
    \file market_handler.h
    \brief Market handler definition
    \author Ivan Shynkarenka
    \date 04.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MARKET_HANDLER_H
#define CPPTRADER_MARKET_HANDLER_H

#include "level.h"
#include "order.h"
#include "order_book.h"
#include "symbol.h"

namespace CppTrader {

//! Market handler class
/*!
    Market handler is used to handle all market events from MarketManager
    with a custom actions. Custom implementations of the market handler
    could be used to monitor the following changes in the market:
    * Add/Remove/Modify symbols
    * Add/Remove/Modify orders
    * Order executions
    * Order book updates

    Not thread-safe.
*/
class MarketHandler
{
    friend class MarketManager;

public:
    MarketHandler() = default;
    MarketHandler(const MarketHandler&) = delete;
    MarketHandler(MarketHandler&&) noexcept = default;
    virtual ~MarketHandler() = default;

    MarketHandler& operator=(const MarketHandler&) = delete;
    MarketHandler& operator=(MarketHandler&&) noexcept = default;

protected:
    // Symbol handlers
    virtual void onAddSymbol(const Symbol& symbol) {}
    virtual void onDeleteSymbol(const Symbol& symbol) {}

    // Order book handlers
    virtual void onAddOrderBook(const OrderBook& order_book) {}
    virtual void onUpdateOrderBook(const OrderBook& order_book, bool top) {}
    virtual void onDeleteOrderBook(const OrderBook& order_book) {}

    // Order handlers
    virtual void onAddOrder(const Order& order) {}
    virtual void onReduceOrder(const Order& order, uint64_t quantity) {}
    virtual void onModifyOrder(const Order& order, uint64_t new_price, uint64_t new_quantity) {}
    virtual void onReplaceOrder(const Order& order, uint64_t new_id, uint64_t new_price, uint64_t new_quantity) {}
    virtual void onReplaceOrder(const Order& order, const Order& new_order) {}
    virtual void onUpdateOrder(const Order& order) {}
    virtual void onDeleteOrder(const Order& order) {}

    // Order execution handlers
    virtual void onExecuteOrder(const Order& order, uint64_t price, uint64_t quantity) {}
};

} // namespace CppTrader

#endif // CPPTRADER_MARKET_HANDLER_H
