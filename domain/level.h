/*!
    \file level.h
    \brief Price level domain model
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_DOMAIN_LEVEL_H
#define CPPTRADER_DOMAIN_LEVEL_H

#include "order.h"

#include "containers/bintree_avl.h"

namespace CppTrader {

//! Price level model
struct Level : public CppCommon::BinTreeAVL<Level>::Node
{
    //! Level price
    uint64_t Price;
    //! Level size
    uint64_t Size;
    //! Level volume
    uint64_t Volume;

    //! Price level orders
    CppCommon::List<Order> Orders;

    Level() noexcept : Level(0, 0, 0) {}
    Level(uint64_t price, uint64_t size, uint64_t volume) noexcept
    {
        Price = price;
        Size = size;
        Volume = volume;
    }
    Level(const Level&) noexcept = default;
    Level(Level&&) noexcept = default;
    ~Level() noexcept = default;

    Level& operator=(const Level&) noexcept = default;
    Level& operator=(Level&&) noexcept = default;

    // Price level comparison
    friend bool operator==(const Level& level1, const Level& level2) noexcept
    { return level1.Price == level2.Price; }
    friend bool operator!=(const Level& level1, const Level& level2) noexcept
    { return level1.Price != level2.Price; }
    friend bool operator<(const Level& level1, const Level& level2) noexcept
    { return level1.Price < level2.Price; }
    friend bool operator>(const Level& level1, const Level& level2) noexcept
    { return level1.Price > level2.Price; }
    friend bool operator<=(const Level& level1, const Level& level2) noexcept
    { return level1.Price <= level2.Price; }
    friend bool operator>=(const Level& level1, const Level& level2) noexcept
    { return level1.Price >= level2.Price; }
};

} // namespace CppTrader

#endif // CPPTRADER_DOMAIN_LEVEL_H
