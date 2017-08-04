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

OrderManager::~OrderManager()
{
    for (auto order_it : _orders)
        _pool.Release(order_it.second);
    _orders.clear();
}

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
    Order* order_ptr = _pool.Create(order);
    _orders[order_ptr->Id] = order_ptr;

    return order_ptr;
}

Order* OrderManager::ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity)
{
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::RuntimeException("Order Id must be greater than zero!");
    assert((new_id > 0) && "Order Id must be greater than zero!");
    if (new_id == 0)
        throwex CppCommon::RuntimeException("Order Id must be greater than zero!");

    auto it = _orders.find(id);
    assert((it != _orders.end()) && "Order not found!");
    if (it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));

    // Replace the order
    Order* order_ptr = it->second;
    _orders.erase(it);
    order_ptr->Id = new_id;
    order_ptr->Price = new_price;
    order_ptr->Quantity = new_quantity;
    _orders[order_ptr->Id] = order_ptr;

    return order_ptr;
}

Order* OrderManager::ReplaceOrder(uint64_t id, const Order& new_order)
{
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::RuntimeException("Order Id must be greater than zero!");
    assert((new_order.Id > 0) && "Order Id must be greater than zero!");
    if (new_order.Id == 0)
        throwex CppCommon::RuntimeException("Order Id must be greater than zero!");

    auto it = _orders.find(id);
    assert((it != _orders.end()) && "Order not found!");
    if (it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));

    // Replace the order
    Order* order_ptr = it->second;
    _orders.erase(it);
    *order_ptr = new_order;
    _orders[order_ptr->Id] = order_ptr;

    return order_ptr;
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
    Order* order_ptr = it->second;
    _orders.erase(it);
    _pool.Release(order_ptr);
}

} // namespace CppTrader
