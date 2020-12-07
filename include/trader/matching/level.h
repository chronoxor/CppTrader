/*!
    \file level.h
    \brief Price level definition
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MATCHING_LEVEL_H
#define CPPTRADER_MATCHING_LEVEL_H

#include "order.h"
#include "update.h"

#include "containers/bintree_avl.h"

namespace CppTrader {
namespace Matching {

//! Price level type
enum class LevelType : uint8_t
{
    BID,
    ASK
};

template <class TOutputStream>
TOutputStream& operator<<(TOutputStream& stream, LevelType type);

//! Price level
struct Level
{
    //! Level type
    LevelType Type;
    //! Level price
    uint64_t Price;
    //! Level volume
    uint64_t TotalVolume;
    //! Level hidden volume
    uint64_t HiddenVolume;
    //! Level visible volume
    uint64_t VisibleVolume;
    //! Level orders
    size_t Orders;

    Level(LevelType type, uint64_t price) noexcept;
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

    template <class TOutputStream>
    friend TOutputStream& operator<<(TOutputStream& stream, const Level& level);

    //! Is the bid price level?
    bool IsBid() const noexcept { return Type == LevelType::BID; }
    //! Is the ask price level?
    bool IsAsk() const noexcept { return Type == LevelType::ASK; }
};

//! Price level node
struct LevelNode : public Level, public CppCommon::BinTreeAVL<LevelNode>::Node
{
    //! Price level orders
    CppCommon::List<OrderNode> OrderList;

    LevelNode(LevelType type, uint64_t price) noexcept;
    LevelNode(const Level& level) noexcept;
    LevelNode(const LevelNode&) noexcept = default;
    LevelNode(LevelNode&&) noexcept = default;
    ~LevelNode() noexcept = default;

    LevelNode& operator=(const Level& level) noexcept;
    LevelNode& operator=(const LevelNode&) noexcept = default;
    LevelNode& operator=(LevelNode&&) noexcept = default;

    // Price level comparison
    friend bool operator==(const LevelNode& level1, const LevelNode& level2) noexcept
    { return level1.Price == level2.Price; }
    friend bool operator!=(const LevelNode& level1, const LevelNode& level2) noexcept
    { return level1.Price != level2.Price; }
    friend bool operator<(const LevelNode& level1, const LevelNode& level2) noexcept
    { return level1.Price < level2.Price; }
    friend bool operator>(const LevelNode& level1, const LevelNode& level2) noexcept
    { return level1.Price > level2.Price; }
    friend bool operator<=(const LevelNode& level1, const LevelNode& level2) noexcept
    { return level1.Price <= level2.Price; }
    friend bool operator>=(const LevelNode& level1, const LevelNode& level2) noexcept
    { return level1.Price >= level2.Price; }
};

//! Price level update
struct LevelUpdate
{
    //! Update type
    UpdateType Type;
    //! Level update value
    Level Update;
    //! Top of the book flag
    bool Top;

    LevelUpdate(UpdateType type, const Level& update, bool top) noexcept;
    LevelUpdate(const LevelUpdate&) noexcept = default;
    LevelUpdate(LevelUpdate&&) noexcept = default;
    ~LevelUpdate() noexcept = default;

    LevelUpdate& operator=(const LevelUpdate&) noexcept = default;
    LevelUpdate& operator=(LevelUpdate&&) noexcept = default;

    template <class TOutputStream>
    friend TOutputStream& operator<<(TOutputStream& stream, const LevelUpdate& update);
};

} // namespace Matching
} // namespace CppTrader

#include "level.inl"

#endif // CPPTRADER_MATCHING_LEVEL_H
