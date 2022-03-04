/*!
    \file market_manager.h
    \brief Market manager definition
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MATCHING_MARKET_MANAGER_H
#define CPPTRADER_MATCHING_MARKET_MANAGER_H

#include "fast_hash.h"
#include "market_handler.h"

#include "containers/hashmap.h"
#include "memory/allocator_pool.h"

#include <cassert>
#include <vector>

namespace CppTrader {

/*!
    \namespace CppTrader::Matching
    \brief Matching engine definitions
*/
namespace Matching {

//! Market manager
/*!
    Market manager is used to manage the market with symbols, orders and order books.

    Automatic orders matching can be enabled with EnableMatching() method or can be
    manually performed with Match() method.

    Not thread-safe.
*/
class MarketManager
{
    friend class OrderBook;

public:
    //! Symbols container
    typedef std::vector<Symbol*> Symbols;
    //! Order books container
    typedef std::vector<OrderBook*> OrderBooks;
    //! Orders container
    typedef CppCommon::HashMap<uint64_t, OrderNode*, FastHash> Orders;

    MarketManager();
    MarketManager(MarketHandler& market_handler);
    MarketManager(const MarketManager&) = delete;
    MarketManager(MarketManager&&) = delete;
    ~MarketManager();

    MarketManager& operator=(const MarketManager&) = delete;
    MarketManager& operator=(MarketManager&&) = delete;

    //! Get the symbols container
    const Symbols& symbols() const noexcept { return _symbols; }
    //! Get the order books container
    const OrderBooks& order_books() const noexcept { return _order_books; }
    //! Get the orders container
    const Orders& orders() const noexcept { return _orders; }

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
    const OrderBook* GetOrderBook(uint32_t id) const noexcept;
    //! Get the order with the given Id
    /*!
        \param id - Order Id
        \return Pointer to the order with the given Id or nullptr
    */
    const Order* GetOrder(uint64_t id) const noexcept;

    //! Add a new symbol
    /*!
        \param symbol - Symbol to add
        \return Error code
    */
    ErrorCode AddSymbol(const Symbol& symbol);
    //! Delete the symbol
    /*!
        \param id - Symbol Id
        \return Error code
    */
    ErrorCode DeleteSymbol(uint32_t id);

    //! Add a new order book
    /*!
        \param symbol - Symbol of the order book to add
        \return Error code
    */
    ErrorCode AddOrderBook(const Symbol& symbol);
    //! Delete the order book
    /*!
        \param id - Symbol Id of the order book
        \return Error code
    */
    ErrorCode DeleteOrderBook(uint32_t id);

    //! Add a new order
    /*!
        \param order - Order to add
        \return Error code
    */
    ErrorCode AddOrder(const Order& order);
    //! Reduce the order by the given quantity
    /*!
        \param id - Order Id
        \param quantity - Order quantity to reduce
        \return Error code
    */
    ErrorCode ReduceOrder(uint64_t id, uint64_t quantity);
    //! Modify the order
    /*!
        Order new quantity will be calculated in a following way:
        \code{.cpp}
        oder.Quantity = new_quantity;
        oder.LeavesQuantity = new_quantity;
        \endcode

        \param id - Order Id
        \param new_price - Order price to modify
        \param new_quantity - Order quantity to modify
        \return Error code
    */
    ErrorCode ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity);
    //! Mitigate the order
    /*!
        The in-flight mitigation functionality prevents an order from being filled
        for a quantity greater than the quantity requested by the user. It protects
        from the risk of a resting order being filled between the time an order
        modification is submitted and the time the order modification is processed
        and applied to the order.

        Order new quantity will be calculated in a following way:
        \code{.cpp}
        if (new_quantity > oder.ExecutedQuantity)
        {
            oder.Quantity = new_quantity;
            oder.LeavesQuantity = new_quantity - oder.ExecutedQuantity;

            // Order will be modified...
        }
        else
        {
            oder.Quantity = new_quantity;
            oder.LeavesQuantity = 0;

            // Order will be canceled...
        }
        \endcode

        \param id - Order Id
        \param new_price - Order price to mitigate
        \param new_quantity - Order quantity to mitigate
        \return Error code
    */
    ErrorCode MitigateOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity);
    //! Replace the order with a similar order but different Id, price and quantity
    /*!
        \param id - Order Id
        \param new_id - Order Id to replace
        \param new_price - Order price to replace
        \param new_quantity - Order quantity to replace
        \return Error code
    */
    ErrorCode ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity);
    //! Replace the order with a new one
    /*!
        \param id - Order Id
        \param new_order - Order to replace
        \return Error code
    */
    ErrorCode ReplaceOrder(uint64_t id, const Order& new_order);
    //! Delete the order
    /*!
        \param id - Order Id
        \return Error code
    */
    ErrorCode DeleteOrder(uint64_t id);

    //! Execute the order
    /*!
        \param id - Order Id
        \param quantity - Order executed quantity
        \return Error code
    */
    ErrorCode ExecuteOrder(uint64_t id, uint64_t quantity);
    //! Execute the order
    /*!
        \param id - Order Id
        \param price - Order executed price
        \param quantity - Order executed quantity
        \return Error code
    */
    ErrorCode ExecuteOrder(uint64_t id, uint64_t price, uint64_t quantity);

    //! Is automatic matching enabled?
    bool IsMatchingEnabled() const noexcept { return _matching; }
    //! Enable automatic matching
    void EnableMatching() { _matching = true; Match(); }
    //! Disable automatic matching
    void DisableMatching() { _matching = false; }

    //! Match crossed orders in all order books
    /*!
        Method will match all crossed orders in each order book. Buy orders will be
        matched with sell orders at arbitrage price starting from the top of the book.
        Matched orders will be executed with deleted form the order book. After the
        matching operation each order book will have the best bid price guarantied
        less than the best ask price!
    */
    void Match();

private:
    // Market handler
    static MarketHandler _default;
    MarketHandler& _market_handler;

    // Auxiliary memory manager
    CppCommon::DefaultMemoryManager _auxiliary_memory_manager;

    // Bid/Ask price levels
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _level_memory_manager;
    CppCommon::PoolAllocator<LevelNode, CppCommon::DefaultMemoryManager> _level_pool;

    // Symbols
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _symbol_memory_manager;
    CppCommon::PoolAllocator<Symbol, CppCommon::DefaultMemoryManager> _symbol_pool;
    Symbols _symbols;

    // Order books
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _order_book_memory_manager;
    CppCommon::PoolAllocator<OrderBook, CppCommon::DefaultMemoryManager> _order_book_pool;
    OrderBooks _order_books;

    // Orders
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _order_memory_manager;
    CppCommon::PoolAllocator<OrderNode, CppCommon::DefaultMemoryManager> _order_pool;
    Orders _orders;

    ErrorCode AddMarketOrder(const Order& order, bool recursive);
    ErrorCode AddLimitOrder(const Order& order, bool recursive);
    ErrorCode AddStopOrder(const Order& order, bool recursive);
    ErrorCode AddStopLimitOrder(const Order& order, bool recursive);
    ErrorCode ReduceOrder(uint64_t id, uint64_t quantity, bool recursive);
    ErrorCode ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity, bool mitigate, bool recursive);
    ErrorCode ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity, bool recursive);
    ErrorCode DeleteOrder(uint64_t id, bool recursive);

    // Matching
    bool _matching;

    void Match(OrderBook* order_book_ptr);
    void MatchMarket(OrderBook* order_book_ptr, Order* order_ptr);
    void MatchLimit(OrderBook* order_book_ptr, Order* order_ptr);
    void MatchOrder(OrderBook* order_book_ptr, Order* order_ptr);

    bool ActivateStopOrders(OrderBook* order_book_ptr);
    bool ActivateStopOrders(OrderBook* order_book_ptr, LevelNode* level_ptr, uint64_t stop_price);
    bool ActivateStopOrder(OrderBook* order_book_ptr, OrderNode* order_ptr);
    bool ActivateStopLimitOrder(OrderBook* order_book_ptr, OrderNode* order_ptr);

    uint64_t CalculateMatchingChain(OrderBook* order_book_ptr, LevelNode* level_ptr, uint64_t price, uint64_t volume);
    uint64_t CalculateMatchingChain(OrderBook* order_book_ptr, LevelNode* bid_level_ptr, LevelNode* ask_level_ptr);
    void ExecuteMatchingChain(OrderBook* order_book_ptr, LevelNode* level_ptr, uint64_t price, uint64_t volume);
    void RecalculateTrailingStopPrice(OrderBook* order_book_ptr, LevelNode* level_ptr);

    void UpdateLevel(const OrderBook& order_book, const LevelUpdate& update) const;
};

/*! \example market_manager.cpp Market manager example */
/*! \example matching_engine.cpp Matching engine example */

} // namespace Matching
} // namespace CppTrader

#include "market_manager.inl"

#endif // CPPTRADER_MATCHING_MARKET_MANAGER_H
