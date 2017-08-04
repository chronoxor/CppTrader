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

MarketManager::~MarketManager()
{
    for (auto order_book_ptr : _order_book)
        if (order_book_ptr != nullptr)
            _pool.Release(order_book_ptr);
    _order_book.clear();
}

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
    OrderBook* order_book_ptr = _pool.Create();
    _order_book[symbol.Id] = order_book_ptr;
}

void MarketManager::DeleteSymbol(uint32_t id)
{
    assert(((id < _order_book.size()) && (_order_book[id] != nullptr)) && "Order book not found!");
    if ((_order_book.size() <= id) || (_order_book[id] == nullptr))
        throwex CppCommon::RuntimeException("Order book not found! Symbol Id = {}"_format(id));

    // Delete the order book
    OrderBook* order_book_ptr = _order_book[id];
    _order_book[id] = nullptr;
    _pool.Release(order_book_ptr);

    // Delete the symbol from the symbol manager
    _symbols.DeleteSymbol(id);
}

void MarketManager::AddOrder(const Order& order)
{
    // Validate parameters
    if (order.Id == 0)
        return;
    if (order.Quantity == 0)
        return;

    // Add the new order into the order manager
    Order* order_ptr = _orders.AddOrder(order);
    if (order_ptr == nullptr)
        return;

    // Get the valid order book for the new order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Add the new order into the order book
        order_book_ptr->AddOrder(order_ptr);
    }
}

void MarketManager::ReduceOrder(uint64_t id, uint64_t quantity)
{
    // Validate parameters
    if (id == 0)
        return;
    if (quantity == 0)
        return;

    // Get the order to reduce from the order manager
    Order* order_ptr = (Order*)_orders.GetOrder(id);
    if (order_ptr == nullptr)
        return;

    // Calculate the minimal possible order quantity to reduce
    quantity = std::min(quantity, order_ptr->Quantity);

    // Get the valid order book for the reducing order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Reduce the order in the order book
        order_book_ptr->ReduceOrder(order_ptr, quantity);
    }

    // Reduce the order quantity
    order_ptr->Quantity -= quantity;

    // Delete the empty order from the order manager
    if (order_ptr->Quantity == 0)
        _orders.DeleteOrder(order_ptr->Id);
}

void MarketManager::ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity)
{
    // Validate parameters
    if (id == 0)
        return;
    if (new_quantity == 0)
        return;

    // Get the order to modify from the order manager
    Order* order_ptr = (Order*)_orders.GetOrder(id);
    if (order_ptr == nullptr)
        return;

    // Get the valid order book for the modifying order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Delete the order from the order book
        order_book_ptr->DeleteOrder(order_ptr);
    }

    // Modify the order
    order_ptr->Price = new_price;
    order_ptr->Quantity = new_quantity;

    if (order_book_ptr != nullptr)
    {
        // Add the modified order into the order book
        order_book_ptr->AddOrder(order_ptr);
    }
}

void MarketManager::ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity)
{
    // Validate parameters
    if (id == 0)
        return;
    if (new_id == 0)
        return;
    if (new_quantity == 0)
        return;

    // Get the order to replace from the order manager
    Order* order_ptr = (Order*)_orders.GetOrder(id);
    if (order_ptr == nullptr)
        return;

    // Get the valid order book for the replacing order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Delete the old order from the order book
        order_book_ptr->DeleteOrder(order_ptr);
    }

    // Replace the order in the order manager
    order_ptr = _orders.ReplaceOrder(id, new_id, new_price, new_quantity);

    if (order_book_ptr != nullptr)
    {
        // Add the modified order into the order book
        order_book_ptr->AddOrder(order_ptr);
    }
}

void MarketManager::ReplaceOrder(uint64_t id, const Order& new_order)
{
    // Validate parameters
    if (id == 0)
        return;
    if (new_order.Id == 0)
        return;
    if (new_order.Quantity == 0)
        return;

    // Get the order to replace from the order manager
    Order* order_ptr = (Order*)_orders.GetOrder(id);
    if (order_ptr == nullptr)
        return;

    // Get the valid order book for the replacing order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Delete the old order from the order book
        order_book_ptr->DeleteOrder(order_ptr);
    }

    // Replace the order in the order manager
    order_ptr = _orders.ReplaceOrder(id, new_order);

    if (order_book_ptr != nullptr)
    {
        // Add the modified order into the order book
        order_book_ptr->AddOrder(order_ptr);
    }
}

void MarketManager::DeleteOrder(uint64_t id)
{
    // Validate parameters
    if (id == 0)
        return;

    // Get the order to delete from the order manager
    Order* order_ptr = (Order*)_orders.GetOrder(id);
    if (order_ptr == nullptr)
        return;

    // Get the valid order book for the deleting order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Delete the order from the order book
        order_book_ptr->DeleteOrder(order_ptr);
    }

    // Delete the order from the order manager
    _orders.DeleteOrder(order_ptr->Id);
}

} // namespace CppTrader
