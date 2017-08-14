/*!
    \file order.h
    \brief Order definition
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MATCHING_ORDER_H
#define CPPTRADER_MATCHING_ORDER_H

#include "containers/list.h"
#include "utility/iostream.h"

#include <cstdint>

namespace CppTrader {
namespace Matching {

//! Order side
enum class OrderSide : uint8_t
{
    BUY,
    SELL
};
std::ostream& operator<<(std::ostream& stream, OrderSide side);

//! Order type
enum class OrderType : uint8_t
{
    LIMIT
};
std::ostream& operator<<(std::ostream& stream, OrderType type);

//! Order
struct Order
{
    //! Order Id
    uint64_t Id;
    //! Symbol Id
    uint32_t SymbolId;
    //! Order type
    OrderType Type;
    //! Order side
    OrderSide Side;
    //! Order price
    uint64_t Price;
    //! Order quantity
    uint64_t Quantity;

    Order(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t quantity) noexcept;
    Order(const Order&) noexcept = default;
    Order(Order&&) noexcept = default;
    ~Order() noexcept = default;

    Order& operator=(const Order&) noexcept = default;
    Order& operator=(Order&&) noexcept = default;

    friend std::ostream& operator<<(std::ostream& stream, const Order& order);
};

struct LevelNode;

//! Order node
struct OrderNode : public Order, public CppCommon::List<OrderNode>::Node
{
    LevelNode* Level;

    OrderNode(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t quantity) noexcept;
    OrderNode(const Order& order) noexcept;
    OrderNode(const OrderNode&) noexcept = default;
    OrderNode(OrderNode&&) noexcept = default;
    ~OrderNode() noexcept = default;

    OrderNode& operator=(const Order& order) noexcept;
    OrderNode& operator=(const OrderNode&) noexcept = default;
    OrderNode& operator=(OrderNode&&) noexcept = default;
};

} // namespace Matching
} // namespace CppTrader

#include "order.inl"

#endif // CPPTRADER_MATCHING_ORDER_H
