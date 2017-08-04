/*!
    \file order_book.h
    \brief Order book definition
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_ORDER_BOOK_H
#define CPPTRADER_ORDER_BOOK_H

#include "../../domain/level.h"

#include "containers/bintree_avl.h"
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

    OrderBook();
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

private:
    CppCommon::DefaultMemoryManager _default_manager;
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _pool_manager;
    CppCommon::PoolAllocator<Level, CppCommon::DefaultMemoryManager> _pool;
    Levels _bids;
    Levels _asks;

    Levels::iterator FindLevel(OrderSide side, uint64_t price) noexcept;

    void AddOrder(Order* order);
    void ReduceOrder(Order* order, uint64_t quantity);
    void DeleteOrder(Order* order);
};

} // namespace CppTrader

#include "order_book.inl"

#endif // CPPTRADER_ORDER_BOOK_H
