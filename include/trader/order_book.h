/*!
    \file order_book.h
    \brief Order book definition
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_ORDER_BOOK_H
#define CPPTRADER_ORDER_BOOK_H

#include "level.h"
#include "symbol.h"

#include "memory/allocator_pool.h"

namespace CppTrader {

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
    typedef CppCommon::BinTreeAVL<Level, std::less<Level>> Levels;

    OrderBook(const Symbol& symbol);
    OrderBook(const OrderBook&) = delete;
    OrderBook(OrderBook&&) noexcept = default;
    ~OrderBook();

    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook& operator=(OrderBook&&) noexcept = default;

    //! Check if the order book is not empty
    explicit operator bool() const noexcept { return !empty(); }

    //! Is the order book empty?
    bool empty() const noexcept { return _bids.empty() && _asks.empty(); }

    //! Get the order book size
    size_t size() const noexcept { return _bids.size() + _asks.size(); }

    //! Get the order book bids container
    const Levels& bids() const noexcept { return _bids; }
    //! Get the order book asks container
    const Levels& asks() const noexcept { return _asks; }

    //! Get the order book symbol
    const Symbol& symbol() const noexcept { return _symbol; }

    friend std::ostream& operator<<(std::ostream& stream, const OrderBook& order_book);

private:
    // Order book symbol
    Symbol _symbol;

    // Auxiliary memory manager
    CppCommon::DefaultMemoryManager _auxiliary_memory_manager;

    // Bid/Ask price levels
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _level_memory_manager;
    CppCommon::PoolAllocator<Level, CppCommon::DefaultMemoryManager> _level_pool;
    Levels _bids;
    Levels _asks;
    Level* _best_bid;
    Level* _best_ask;

    Levels::iterator FindLevel(OrderSide side, uint64_t price) noexcept;

    Level* AddLevel(Order* order_ptr);
    Level* DeleteLevel(Order* order_ptr, Level* level_ptr);

    std::pair<Level*, bool> AddOrder(Order* order_ptr);
    std::pair<Level*, bool> ReduceOrder(Order* order_ptr, uint64_t quantity);
    std::pair<Level*, bool> DeleteOrder(Order* order_ptr);
};

} // namespace CppTrader

#include "order_book.inl"

#endif // CPPTRADER_ORDER_BOOK_H
