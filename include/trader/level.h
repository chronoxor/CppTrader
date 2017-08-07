/*!
    \file level.h
    \brief Price level definition
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_LEVEL_H
#define CPPTRADER_LEVEL_H

#include "order.h"

#include "containers/bintree_avl.h"

namespace CppTrader {

//! Price level
struct Level : public CppCommon::BinTreeAVL<Level>::Node
{
    //! Level price
    uint64_t Price;
    //! Level volume
    uint64_t Volume;

    //! Price level orders
    CppCommon::List<Order> Orders;

    Level(uint64_t price) noexcept;
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

    friend std::ostream& operator<<(std::ostream& stream, const Level& level);
};

} // namespace CppTrader

#include "level.inl"

#endif // CPPTRADER_LEVEL_H
