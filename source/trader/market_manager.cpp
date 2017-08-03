/*!
    \file market_manager.cpp
    \brief Market manager implementation
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

#include "trader/market_manager.h"

#include "errors/exceptions.h"
#include "string/format.h"

namespace CppTrader {

void MarketManager::AddSymbol(const Symbol& symbol)
{
    // Add the new symbol into the symbol manager
    _symbols.AddSymbol(symbol);

    // Resize the order book container
    if (_order_book.size() <= symbol.Id)
        _order_book.resize(symbol.Id + 1, nullptr);

    assert((_order_book[symbol.Id] == nullptr) && "Duplicate order book detected!");
    if (_order_book[symbol.Id] != nullptr)
        throwex CppCommon::RuntimeException("Duplicate order book detected! Symbol Id = {}"_format(symbol.Id));

    // Add the order book
    OrderBook* order_book = _pool.Create();
    _order_book[symbol.Id] = order_book;
}

void MarketManager::DeleteSymbol(uint32_t id)
{
    assert(((id < _order_book.size()) && (_order_book[id] != nullptr)) && "Order book not found!");
    if ((_order_book.size() <= id) || (_order_book[id] == nullptr))
        throwex CppCommon::RuntimeException("Order book not found! Symbol Id = {}"_format(id));

    // Delete the order book
    OrderBook* order_book = _order_book[id];
    _order_book[id] = nullptr;
    _pool.Release(order_book);

    // Delete the symbol from the symbol manager
    _symbols.DeleteSymbol(id);
}

void MarketManager::AddOrder(const Order& order)
{
    // Add the new order into the order manager
    Order* new_order = _orders.AddOrder(order);
    if (new_order == nullptr)
        return;

    // Get the valid order book for the new order
    OrderBook* order_book = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book != nullptr)
    {
        // Add the new order into the order book
        order_book->AddOrder(new_order);
    }
}

void MarketManager::DeleteOrder(uint64_t id)
{
    // Get the order to delete from the order manager
    Order* order = (Order*)_orders.GetOrder(id);
    if (order == nullptr)
        return;

    // Get the valid order book for the deleting order
    OrderBook* order_book = (OrderBook*)GetOrderBook(order->SymbolId);
    if (order_book != nullptr)
    {
        // Delete the order from the order book
        order_book->DeleteOrder(order);
    }

    // Delete the order from the order manager
    _orders.DeleteOrder(order->Id);
}

} // namespace CppTrader
