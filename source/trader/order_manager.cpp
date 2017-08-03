/*!
    \file order_manager.cpp
    \brief Order manager implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#include "trader/order_manager.h"

namespace CppTrader {

bool OrderManager::RegisterOrder(const Order& order)
{
    uint64_t id = order.Id + 1;

    // Check if the order with a given Id is already registered
    if (_orders.find(id) != _orders.end())
        return false;

    // Register order
    Order* result = _pool.Create(order);
    _orders[id] = result;
    return true;
}

bool OrderManager::UnregisterOrder(uint64_t id)
{
    auto it = _orders.find(id + 1);

    // Check if the order with a given Id is registered before
    if (it == _orders.end())
        return false;

    // Unregister order
    Order* result = it->second;
    _orders.erase(it);
    _pool.Release(result);
    return true;
}

} // namespace CppTrader
