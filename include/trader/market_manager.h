/*!
    \file market_manager.h
    \brief Market manager definition
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MARKET_MANAGER_H
#define CPPTRADER_MARKET_MANAGER_H

#include "fast_hash.h"
#include "market_handler.h"

#include "containers/hashmap.h"
#include "memory/allocator_pool.h"

#include <cassert>
#include <vector>

namespace CppTrader {

//! Market manager
/*!
    Market manager is used to manage the market with symbols, orders and order books.

    Not thread-safe.
*/
class MarketManager
{
public:
    MarketManager();
    MarketManager(MarketHandler& market_handler);
    MarketManager(const MarketManager&) = delete;
    MarketManager(MarketManager&&) = default;
    ~MarketManager();

    MarketManager& operator=(const MarketManager&) = delete;
    MarketManager& operator=(MarketManager&&) = default;

    //! Get the symbol with the given Id
    /*!
        \param id - Symbol Id
        \return Pointer to the symobl with the given Id or nullptr
    */
    const Symbol* GetSymbol(uint32_t id) const noexcept;
    //! Get the order book for the given symbol Id
    /*!
        \param id - Symbol Id of the order book
        \return Pointer to the order book with the given symbol Id or nullptr
    */
    const OrderBook* GetOrderBook(uint32_t symbol) const noexcept;
    //! Get the order with the given Id
    /*!
        \param id - Order Id
        \return Pointer to the order with the given Id or nullptr
    */
    const Order* GetOrder(uint64_t id) const noexcept;

    //! Add a new symbol
    /*!
        \param symbol - Symbol to add
    */
    void AddSymbol(const Symbol& symbol);
    //! Delete the symbol
    /*!
        \param id - Symbol Id
    */
    void DeleteSymbol(uint32_t id);

    //! Add a new order book
    /*!
        \param symbol - Symbol of the order book to add
    */
    void AddOrderBook(const Symbol& symbol);
    //! Delete the order book
    /*!
        \param id - Symbol Id of the order book
    */
    void DeleteOrderBook(uint32_t id);

    //! Add a new order
    /*!
        \param order - Order to add
    */
    void AddOrder(const Order& order);
    //! Reduce the order by the given quantity
    /*!
        \param id - Order Id
        \param quantity - Order quantity to reduce
    */
    void ReduceOrder(uint64_t id, uint64_t quantity);
    //! Modify the order
    /*!
        \param id - Order Id
        \param new_price - Order price to modify
        \param new_quantity - Order quantity to modify
    */
    void ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity);
    //! Replace the order with a similar order but different Id, price and quantity
    /*!
        \param id - Order Id
        \param new_id - Order Id to replace
        \param new_price - Order price to replace
        \param new_quantity - Order quantity to replace
    */
    void ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity);
    //! Replace the order with a new one
    /*!
        \param id - Order Id
        \param new_order - Order to replace
    */
    void ReplaceOrder(uint64_t id, const Order& new_order);
    //! Delete the order
    /*!
        \param id - Order Id
    */
    void DeleteOrder(uint64_t id);

    //! Execute the order
    /*!
        \param id - Order Id
        \param quantity - Order executed quantity
    */
    void ExecuteOrder(uint64_t id, uint64_t quantity);
    //! Execute the order
    /*!
        \param id - Order Id
        \param price - Order executed price
        \param quantity - Order executed quantity
    */
    void ExecuteOrder(uint64_t id, uint64_t price, uint64_t quantity);

private:
    // Market handler
    static MarketHandler _default;
    MarketHandler& _market_handler;

    // Auxiliary memory manager
    CppCommon::DefaultMemoryManager _auxiliary_memory_manager;

    // Symbols
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _symbol_memory_manager;
    CppCommon::PoolAllocator<Symbol, CppCommon::DefaultMemoryManager> _symbol_pool;
    std::vector<Symbol*> _symbols;

    // Order books
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _order_book_memory_manager;
    CppCommon::PoolAllocator<OrderBook, CppCommon::DefaultMemoryManager> _order_book_pool;
    std::vector<OrderBook*> _order_books;

    // Orders
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _order_memory_manager;
    CppCommon::PoolAllocator<Order, CppCommon::DefaultMemoryManager> _order_pool;
    CppCommon::HashMap<uint64_t, Order*, FastHash> _orders;

    void UpdateOrderBook(const OrderBook& order_book, const std::pair<Level*, bool>& order_book_update1, const std::pair<Level*, bool>& order_book_update2 = std::make_pair(nullptr, false));
};

/*! \example market_manager.cpp Market manager example */

} // namespace CppTrader

#include "market_manager.inl"

#endif // CPPTRADER_MARKET_MANAGER_H
