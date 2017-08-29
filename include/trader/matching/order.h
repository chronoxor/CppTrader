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
#include <limits>

namespace CppTrader {
namespace Matching {

//! Order side
/*!
    Possible values:
    * <b>Buy</b> -  Buy side is the side of orders made up of investing institutions
      such as mutual funds, pension funds and insurance firms that tend to buy large
      portions of securities for money-management purposes.
    * <b>Sell</b> - The opposite side to buy made up of investing institutions that
      tend to sell large portions of securities for money-management purposes.
*/
enum class OrderSide : uint8_t
{
    BUY,
    SELL
};
std::ostream& operator<<(std::ostream& stream, OrderSide side);

//! Order type
/*!
    Possible values:
    * <b>Market order</b> - A market order is an order to buy or sell a stock at the best
      available price. Generally, this type of order will be executed immediately. However,
      the price at which a market order will be executed is not guaranteed. It is important
      for investors to remember that the last-traded price is not necessarily the price at
      which a market order will be executed. In fast-moving markets, the price at which a
      market order will execute often deviates from the last-traded price or "real time"
      quote.
    * <b>Limit order</b> - A limit order is an order to buy or sell a stock at a specific
      price or better. A buy limit order can only be executed at the limit price or lower,
      and a sell limit order can only be executed at the limit price or higher. A limit
      order is not guaranteed to execute. A limit order can only be filled if the stock’s
      market price reaches the limit price. While limit orders do not guarantee execution,
      they help ensure that an investor does not pay more than a predetermined price for a
      stock.
*/
enum class OrderType : uint8_t
{
    MARKET,
    LIMIT
};
std::ostream& operator<<(std::ostream& stream, OrderType type);

//! Order
/*!
    An order is an instruction to buy or sell on a trading venue such as a stock market,
    bond market, commodity market, or financial derivative market. These instructions can
    be simple or complicated, and can be sent to either a broker or directly to a trading
    venue via direct market access.
*/
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

    //! Market order slippage
    uint64_t Slippage;

    Order() noexcept = default;
    Order(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t quantity, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    Order(const Order&) noexcept = default;
    Order(Order&&) noexcept = default;
    ~Order() noexcept = default;

    Order& operator=(const Order&) noexcept = default;
    Order& operator=(Order&&) noexcept = default;

    friend std::ostream& operator<<(std::ostream& stream, const Order& order);

    //! Prepare a new limit order
    static Order Limit(uint64_t id, uint32_t symbol, OrderSide side, uint64_t price, uint64_t quantity) noexcept;
    //! Prepare a new buy limit order
    static Order BuyLimit(uint64_t id, uint32_t symbol, uint64_t price, uint64_t quantity) noexcept;
    //! Prepare a new sell limit order
    static Order SellLimit(uint64_t id, uint32_t symbol, uint64_t price, uint64_t quantity) noexcept;
    //! Prepare a new market order
    static Order Market(uint64_t id, uint32_t symbol, OrderSide side, uint64_t quantity, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new buy market order
    static Order BuyMarket(uint64_t id, uint32_t symbol, uint64_t quantity, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new sell market order
    static Order SellMarket(uint64_t id, uint32_t symbol, uint64_t quantity, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
};

struct LevelNode;

//! Order node
struct OrderNode : public Order, public CppCommon::List<OrderNode>::Node
{
    LevelNode* Level;

    OrderNode(const Order& order) noexcept;
    OrderNode(const OrderNode&) noexcept = default;
    OrderNode(OrderNode&&) noexcept = default;
    ~OrderNode() noexcept = default;

    OrderNode& operator=(const OrderNode&) noexcept = default;
    OrderNode& operator=(OrderNode&&) noexcept = default;
};

} // namespace Matching
} // namespace CppTrader

#include "order.inl"

#endif // CPPTRADER_MATCHING_ORDER_H
