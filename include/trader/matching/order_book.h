/*!
    \file order_book.h
    \brief Order book definition
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MATCHING_ORDER_BOOK_H
#define CPPTRADER_MATCHING_ORDER_BOOK_H

#include "level.h"
#include "symbol.h"

#include "memory/allocator_pool.h"

namespace CppTrader {
namespace Matching {

//! Order book
/*!
    Order book is used to keep buy and sell orders in a price level order.

    Not thread-safe.
*/
class OrderBook
{
    friend class MarketManager;

public:
    //! Price level container
    typedef CppCommon::BinTreeAVL<LevelNode, std::less<LevelNode>> Levels;

    OrderBook(const Symbol& symbol);
    OrderBook(const OrderBook&) = delete;
    OrderBook(OrderBook&&) noexcept = default;
    ~OrderBook();

    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook& operator=(OrderBook&&) noexcept = default;

    //! Check if the order book is not empty
    explicit operator bool() const noexcept { return !empty(); }

    //! Is the order book empty?
    bool empty() const noexcept { return size() == 0; }

    //! Get the order book size
    size_t size() const noexcept { return _bids.size() + _asks.size() + _buy_stop.size() + _sell_stop.size(); }

    //! Get the order book symbol
    const Symbol& symbol() const noexcept { return _symbol; }

    //! Get the order book best bid price level
    const LevelNode* best_bid() const noexcept { return _best_bid; }
    //! Get the order book best ask price level
    const LevelNode* best_ask() const noexcept { return _best_ask; }

    //! Get the order book bids container
    const Levels& bids() const noexcept { return _bids; }
    //! Get the order book asks container
    const Levels& asks() const noexcept { return _asks; }

    //! Get the order book buy stop orders container
    const Levels& buy_stop() const noexcept { return _buy_stop; }
    //! Get the order book sell stop orders container
    const Levels& sell_stop() const noexcept { return _sell_stop; }

    friend std::ostream& operator<<(std::ostream& stream, const OrderBook& order_book);

    //! Get the order book bid price level with the given price
    /*!
        \param price - Price
        \return Pointer to the order book bid price level with the given price or nullptr
    */
    const LevelNode* GetBid(uint64_t price) const noexcept;
    //! Get the order book ask price level with the given price
    /*!
        \param price - Price
        \return Pointer to the order book ask price level with the given price or nullptr
    */
    const LevelNode* GetAsk(uint64_t price) const noexcept;

    //! Get the order book buy stop level with the given price
    /*!
        \param price - Price
        \return Pointer to the order book buy stop level with the given price or nullptr
    */
    const LevelNode* GetBuyStopLevel(uint64_t price) const noexcept;
    //! Get the order book sell stop level with the given price
    /*!
        \param price - Price
        \return Pointer to the order book sell stop level with the given price or nullptr
    */
    const LevelNode* GetSellStopLevel(uint64_t price) const noexcept;

private:
    // Order book symbol
    Symbol _symbol;

    // Auxiliary memory manager
    CppCommon::DefaultMemoryManager _auxiliary_memory_manager;

    // Bid/Ask price levels
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _level_memory_manager;
    CppCommon::PoolAllocator<LevelNode, CppCommon::DefaultMemoryManager> _level_pool;
    LevelNode* _best_bid;
    LevelNode* _best_ask;
    Levels _bids;
    Levels _asks;

    // Price level management
    LevelNode* GetNextLevel(LevelNode* level) noexcept;
    LevelNode* AddLevel(OrderNode* order_ptr);
    LevelNode* DeleteLevel(OrderNode* order_ptr);

    // Orders management
    LevelUpdate AddOrder(OrderNode* order_ptr);
    LevelUpdate ReduceOrder(OrderNode* order_ptr, uint64_t quantity, uint64_t hidden, uint64_t visible);
    LevelUpdate DeleteOrder(OrderNode* order_ptr);

    // Buy/Sell stop orders levels
    Levels _buy_stop;
    Levels _sell_stop;

    // Stop orders level management
    LevelNode* GetNextStopLevel(LevelNode* level) noexcept;
    LevelNode* AddStopLevel(OrderNode* order_ptr);
    LevelNode* DeleteStopLevel(OrderNode* order_ptr);

    // Stop orders management
    void AddStopOrder(OrderNode* order_ptr);
    void ReduceStopOrder(OrderNode* order_ptr, uint64_t quantity, uint64_t hidden, uint64_t visible);
    void DeleteStopOrder(OrderNode* order_ptr);
};

} // namespace Matching
} // namespace CppTrader

#include "order_book.inl"

#endif // CPPTRADER_MATCHING_ORDER_BOOK_H
