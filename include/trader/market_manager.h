/*!
    \file market_manager.h
    \brief Market manager definition
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MARKET_MANAGER_H
#define CPPTRADER_MARKET_MANAGER_H

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
    MarketManager(const MarketManager&) = delete;
    MarketManager(MarketManager&&) noexcept = default;
    ~MarketManager();

    MarketManager& operator=(const MarketManager&) = delete;
    MarketManager& operator=(MarketManager&&) noexcept = default;

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
    //! Delete the symbol with the given Id
    /*!
        \param id - Symbol Id
    */
    void DeleteSymbol(uint32_t id);

    //! Add a new order
    /*!
        \param order - Order to add
    */
    void AddOrder(const Order& order);
    //! Delete the order with the given Id
    /*!
        \param id - Order Id
    */
    void DeleteOrder(uint64_t id);

private:
    CppCommon::DefaultMemoryManager _default_manager;
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _pool_manager;
    CppCommon::PoolAllocator<OrderBook, CppCommon::DefaultMemoryManager> _pool;
    SymbolManager _symbols;
    OrderManager _orders;
    std::vector<OrderBook*> _order_book;
};

} // namespace CppTrader

#include "market_manager.inl"

#endif // CPPTRADER_MARKET_MANAGER_H
