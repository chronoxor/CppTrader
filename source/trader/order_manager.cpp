/*!
    \file order_manager.cpp
    \brief Order manager implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#include "trader/order_manager.h"

namespace CppTrader {

bool OrderManager::AddOrder(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t quantity)
{
    // Check if the order with a given Id is already added
    if (_orders.find(id + 1) != _orders.end())
        return false;

    // Add the order
    Order* result = _pool.Create(id + 1, symbol, type, side, price, quantity);
    _orders[id + 1] = result;

    return true;
}

bool OrderManager::RemoveOrder(uint64_t id)
{
    auto it = _orders.find(id + 1);

    // Check if the order with a given Id is added before
    if (it == _orders.end())
        return false;

    // Remove the order
    Order* result = it->second;
    _orders.erase(it);
    _pool.Release(result);

    return true;
}

} // namespace CppTrader
