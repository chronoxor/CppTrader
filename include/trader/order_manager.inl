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
    auto it = _orders.find(id + 1);
    return ((it != _orders.end()) ? it->second : nullptr);
}

} // namespace CppTrader
