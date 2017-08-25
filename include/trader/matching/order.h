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
    An order is an instruction to buy or sell on a trading venue such as a stock market,
    bond market, commodity market, or financial derivative market. These instructions can
    be simple or complicated, and can be sent to either a broker or directly to a trading
    venue via direct market access.

    Possible values:
    * <b>Market order</b> - A market order is a buy or sell order to be executed immediately
      at current market prices. As long as there are willing sellers and buyers, market orders
      are filled. Market orders are therefore used when certainty of execution is a priority
      over price of execution. A market order is the simplest of the order types. This order
      type does not allow any control over the price received. The order is filled at the best
      price available at the relevant time. In fast-moving markets, the price paid or received
      may be quite different from the last price quoted before the order was entered. A market
      order may be split across multiple participants on the other side of the transaction,
      resulting in different prices for some of the shares.
    * <b>Limit order</b> - A limit order is an order to buy a security at no more than a specific
      price, or to sell a security at no less than a specific price (called "or better" for either
      direction). This gives the trader (customer) control over the price at which the trade is
      executed; however, the order may never be executed ("filled"). Limit orders are used when
      the trader wishes to control price rather than certainty of execution. A <b>buy limit order</b>
      can only be executed at the limit price or lower. For example, if an investor wants to buy
      a stock, but doesn't want to pay more than $20 for it, the investor can place a limit order
      to buy the stock at $20. By entering a limit order rather than a market order, the investor
      will not buy the stock at a higher price, but, may get fewer shares than he wants or not get
      the stock at all. A <b>sell limit order</b> is analogous; it can only be executed at the limit
      price or higher.
*/
enum class OrderType : uint8_t
{
    MARKET,
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

    //! Market order slippage
    uint64_t Slippage;

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
