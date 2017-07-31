/*!
    \file order_manager.inl
    \brief Order manager inline implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

namespace CppTrader {

inline OrderManager::OrderManager()
    : _default_manager(),
      _pool_manager(_default_manager),
      _pool(_pool_manager),
      _orders(16384, 0)
{
}

inline const Order* OrderManager::GetOrder(uint64_t id) const noexcept
{
    auto it = _orders.find(id);
    return ((it != _orders.end()) ? it->second : nullptr);
}

inline bool OrderManager::RegisterOrder(const Order& order)
{
    // Check if the order with a given Id is already registered
    if (_orders.find(order.Id) != _orders.end())
        return false;

    // Register order
    Order* result = _pool.Create(order);
    _orders[result->Id] = result;
    return true;
}

inline bool OrderManager::UnregisterOrder(uint64_t id)
{
    auto it = _orders.find(id);

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
