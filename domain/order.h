/*!
    \file order.h
    \brief Order domain model
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_DOMAIN_ORDER_H
#define CPPTRADER_DOMAIN_ORDER_H

#include <cstdint>

namespace CppTrader {

//! Order side
enum class OrderSide : uint8_t
{
    BUY,
    SELL
};

//! Order type
enum class OrderType : uint8_t
{
    LIMIT
};

//! Order model
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

    Order() noexcept : Order(0, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0) {}
    Order(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t quantity) noexcept
    {
        Id = id;
        SymbolId = symbol;
        Type = type;
        Side = side;
        Price = price;
        Quantity = quantity;
    }
    Order(const Order&) noexcept = default;
    Order(Order&&) noexcept = default;
    ~Order() noexcept = default;

    Order& operator=(const Order&) noexcept = default;
    Order& operator=(Order&&) noexcept = default;
};

} // namespace CppTrader

#endif // CPPTRADER_DOMAIN_ORDER_H
