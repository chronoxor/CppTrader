/*!
    \file market_manager.cpp
    \brief Market manager implementation
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

#include "trader/market_manager.h"

namespace CppTrader {

bool MarketManager::AddSymbol(uint32_t id, const char name[8])
{
    // Add a new symbol in the symbol manager
    if (!_symbols.AddSymbol(id, name))
        return false;

    // Resize the order book container
    if (_order_book.size() <= id)
        _order_book.resize(id + 1, nullptr);

    // Check if the order book with a given symbol Id is already added
    if (_order_book[id] != nullptr)
        return false;

    // Add the order book
    OrderBook* result = _pool.Create();
    _order_book[id] = result;

    return true;
}

bool MarketManager::RemoveSymbol(uint32_t id)
{
    // Check if the order book with a given symbol Id is added before
    if ((_order_book.size() <= id) || (_order_book[id] == nullptr))
        return false;

    // Remove the order book
    OrderBook* result = _order_book[id];
    _order_book[id] = nullptr;
    _pool.Release(result);

    // Remove the symbol from the symbol manager
    if (!_symbols.RemoveSymbol(id))
        return false;

    return true;
}

} // namespace CppTrader
