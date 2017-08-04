/*!
    \file market_manager.h
    \brief Market manager definition
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MARKET_MANAGER_H
#define CPPTRADER_MARKET_MANAGER_H

#include "market_handler.h"
#include "order_book.h"
#include "order_manager.h"
#include "symbol_manager.h"

namespace CppTrader {

//! Market manager
/*!
    Market manager is used to manage the whole market entities - symbols, symbol groups,
    orders and order books.

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

    //! Get the symbol manager
    const SymbolManager& symbols() const noexcept { return _symbols; }
    //! Get the order manager
    const OrderManager& orders() const noexcept { return _orders; }

    //! Get the order book for the given symbol Id
    /*!
        \param symbol - Symbol Id
        \return Pointer to the order book with the given symbol Id or nullptr
    */
    const OrderBook* GetOrderBook(uint32_t symbol) const noexcept;

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
    static MarketHandler _default_handler;
    MarketHandler& _market_handler;
    CppCommon::DefaultMemoryManager _default_manager;
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _pool_manager;
    CppCommon::PoolAllocator<OrderBook, CppCommon::DefaultMemoryManager> _pool;
    SymbolManager _symbols;
    OrderManager _orders;
    std::vector<OrderBook*> _order_book;
};

/*! \example market_manager.cpp Market manager example */

} // namespace CppTrader

#include "market_manager.inl"

#endif // CPPTRADER_MARKET_MANAGER_H
