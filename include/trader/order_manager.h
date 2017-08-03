/*!
    \file order_manager.h
    \brief Order manager definition
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_ORDER_MANAGER_H
#define CPPTRADER_ORDER_MANAGER_H

#include "../../domain/order.h"

#include "fast_hash.h"

#include "containers/hashmap.h"
#include "memory/allocator_pool.h"

namespace CppTrader {

//! Order manager
/*!
    Order manager is used to register and manage orders.

    Not thread-safe.
*/
class OrderManager
{
public:
    OrderManager();
    OrderManager(const OrderManager&) = delete;
    OrderManager(OrderManager&&) noexcept = default;
    ~OrderManager() = default;

    OrderManager& operator=(const OrderManager&) = delete;
    OrderManager& operator=(OrderManager&&) noexcept = default;

    //! Check if the order manager is not empty
    explicit operator bool() const noexcept { return !empty(); }

    //! Get the order with the given Id
    const Order* operator[](uint64_t id) { return GetOrder(id); }

    //! Is the order manager empty?
    bool empty() const noexcept { return _orders.empty(); }

    //! Get the order manager size
    size_t size() const noexcept { return _orders.size(); }

    //! Get the order with the given Id
    /*!
        \param id - Order Id
        \return Pointer to the order with the given Id or nullptr
    */
    const Order* GetOrder(uint64_t id) const noexcept;

    //! Add a new order
    /*!
        \param id - Order Id
        \param symbol - Symbol Id
        \param type - Order type
        \param side - Order side
        \param price - Order price
        \param quantity - Order quantity
        \return 'true' if the order was successfully added, 'false' if the order failed to add
    */
    bool AddOrder(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t quantity);
    //! Remove the order with the given Id
    /*!
        \param id - Order Id
        \return 'true' if the order was successfully removed, 'false' if the order failed to remove
    */
    bool RemoveOrder(uint64_t id);

private:
    CppCommon::DefaultMemoryManager _default_manager;
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _pool_manager;
    CppCommon::PoolAllocator<Order, CppCommon::DefaultMemoryManager> _pool;
    CppCommon::HashMap<uint64_t, Order*, FastHash> _orders;
};

} // namespace CppTrader

#include "order_manager.inl"

#endif // CPPTRADER_ORDER_MANAGER_H
