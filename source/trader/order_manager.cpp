/*!
    \file order_manager.cpp
    \brief Order manager implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#include "trader/order_manager.h"

#include "errors/exceptions.h"
#include "string/format.h"

namespace CppTrader {

Order* OrderManager::AddOrder(const Order& order)
{
    assert((order.Id > 0) && "Order Id must be greater than zero!");
    if (order.Id == 0)
        throwex CppCommon::RuntimeException("Order Id must be greater than zero!");

    auto it = _orders.find(order.Id);
    assert((it == _orders.end()) && "Duplicate symbol detected!");
    if (it != _orders.end())
        throwex CppCommon::RuntimeException("Duplicate order detected! Order Id = {}"_format(order.Id));

    // Add the order
    Order* new_order = _pool.Create(order);
    _orders[order.Id] = new_order;

    return new_order;
}

void OrderManager::DeleteOrder(uint64_t id)
{
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::RuntimeException("Order Id must be greater than zero!");

    auto it = _orders.find(id);
    assert((it != _orders.end()) && "Order not found!");
    if (it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));

    // Delete the order
    Order* order = it->second;
    _orders.erase(it);
    _pool.Release(order);
}

} // namespace CppTrader
