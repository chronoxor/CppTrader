/*!
    \file order.h
    \brief Order definition
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_ORDER_H
#define CPPTRADER_ORDER_H

#include "containers/list.h"
#include "utility/iostream.h"

#include <cstdint>

namespace CppTrader {

struct Level;
class OrderBook;

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
struct Order : public CppCommon::List<Order>::Node
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

    //! Price level
    Level* Level;
    //! Order book
    OrderBook* OrderBook;

    Order(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t quantity) noexcept;
    Order(const Order&) noexcept = default;
    Order(Order&&) noexcept = default;
    ~Order() noexcept = default;

    Order& operator=(const Order&) noexcept = default;
    Order& operator=(Order&&) noexcept = default;

    friend std::ostream& operator<<(std::ostream& stream, const Order& order);
};

} // namespace CppTrader

#include "order.inl"

#endif // CPPTRADER_ORDER_H
