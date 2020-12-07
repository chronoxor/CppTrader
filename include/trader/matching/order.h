/*!
    \file order.h
    \brief Order definition
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_MATCHING_ORDER_H
#define CPPTRADER_MATCHING_ORDER_H

#include "errors.h"

#include "containers/list.h"
#include "utility/iostream.h"

#include <algorithm>
#include <cstdint>
#include <limits>

namespace CppTrader {
namespace Matching {

//! Order side
/*!
    Possible values:
    \li <b>Buy</b> -  Buy side is the side of orders made up of investing institutions
        such as mutual funds, pension funds and insurance firms that tend to buy large
        portions of securities for money-management purposes.
    \li <b>Sell</b> - The opposite side to buy made up of investing institutions that
        tend to sell large portions of securities for money-management purposes.
*/
enum class OrderSide : uint8_t
{
    BUY,
    SELL
};

template <class TOutputStream>
TOutputStream& operator<<(TOutputStream& stream, OrderSide side);

//! Order type
/*!
    Possible values:
    \li <b>Market order</b> - A market order is an order to buy or sell a stock at the best
        available price. Generally, this type of order will be executed immediately. However,
        the price at which a market order will be executed is not guaranteed. It is important
        for investors to remember that the last-traded price is not necessarily the price at
        which a market order will be executed. In fast-moving markets, the price at which a
        market order will execute often deviates from the last-traded price or "real time"
        quote.
    \li <b>Limit order</b> - A limit order is an order to buy or sell a stock at a specific
        price or better. A buy limit order can only be executed at the limit price or lower,
        and a sell limit order can only be executed at the limit price or higher. A limit
        order is not guaranteed to execute. A limit order can only be filled if the stock's
        market price reaches the limit price. While limit orders do not guarantee execution,
        they help ensure that an investor does not pay more than a predetermined price for a
        stock.
    \li <b>Stop order</b> - A stop order, also referred to as a stop-loss order, is an order
        to buy or sell a stock once the price of the stock reaches a specified price, known
        as the stop price. When the stop price is reached, a stop order becomes a market order.
        A buy stop order is entered at a stop price above the current market price. Investors
        generally use a buy stop order to limit a loss or to protect a profit on a stock that
        they have sold short. A sell stop order is entered at a stop price below the current
        market price. Investors generally use a sell stop order to limit a loss or to protect
        a profit on a stock that they own.
    \li <b>Stop-limit order</b> - A stop-limit order is an order to buy or sell a stock that
        combines the features of a stop order and a limit order. Once the stop price is reached,
        a stop-limit order becomes a limit order that will be executed at a specified price (or
        better). The benefit of a stop-limit order is that the investor can control the price at
        which the order can be executed.
    \li <b>Trailing stop order</b> - A trailing stop order is entered with a stop parameter
        that creates a moving or trailing activation price, hence the name. This parameter
        is entered as a percentage change or actual specific amount of rise (or fall) in the
        security price. Trailing stop sell orders are used to maximize and protect profit as
        a stock's price rises and limit losses when its price falls.
    \li <b>Trailing stop-limit order</b> - A trailing stop-limit order is similar to a trailing
        stop order. Instead of selling at market price when triggered, the order becomes a limit
        order.
*/
enum class OrderType : uint8_t
{
    MARKET,
    LIMIT,
    STOP,
    STOP_LIMIT,
    TRAILING_STOP,
    TRAILING_STOP_LIMIT
};

template <class TOutputStream>
TOutputStream& operator<<(TOutputStream& stream, OrderType type);

//! Order Time in Force
/*!
    Possible values:
    \li <b>Good-Till-Cancelled (GTC)</b> - A GTC order is an order to buy or sell a stock that
        lasts until the order is completed or cancelled.
    \li <b>Immediate-Or-Cancel (IOC)</b> - An IOC order is an order to buy or sell a stock that
        must be executed immediately. Any portion of the order that cannot be filled immediately
        will be cancelled.
    \li <b>Fill-Or-Kill (FOK)</b> - An FOK order is an order to buy or sell a stock that must
        be executed immediately in its entirety; otherwise, the entire order will be cancelled
        (i.e., no partial execution of the order is allowed).
    \li <b>All-Or-None (AON)</b> - An All-Or-None (AON) order is an order to buy or sell a stock
        that must be executed in its entirety, or not executed at all. AON orders that cannot be
        executed immediately remain active until they are executed or cancelled.
*/
enum class OrderTimeInForce : uint8_t
{
    GTC,    //!< Good-Till-Cancelled
    IOC,    //!< Immediate-Or-Cancel
    FOK,    //!< Fill-Or-Kill
    AON     //!< All-Or-None
};

template <class TOutputStream>
TOutputStream& operator<<(TOutputStream& stream, OrderTimeInForce tif);

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
    //! Order stop price
    uint64_t StopPrice;

    //! Order quantity
    uint64_t Quantity;
    //! Order executed quantity
    uint64_t ExecutedQuantity;
    //! Order leaves quantity
    uint64_t LeavesQuantity;

    //! Time in Force
    OrderTimeInForce TimeInForce;

    //! Order max visible quantity
    /*!
        This property allows to prepare 'iceberg'/'hidden' orders with the
        following rules:
        \li <b>MaxVisibleQuantity >= LeavesQuantity</b> - Regular order
        \li <b>MaxVisibleQuantity == 0</b> - 'Hidden' order
        \li <b>MaxVisibleQuantity < LeavesQuantity</b> - 'Iceberg' order

        Supported only for limit and stop-limit orders!
    */
    uint64_t MaxVisibleQuantity;
    //! Order hidden quantity
    uint64_t HiddenQuantity() const noexcept { return (LeavesQuantity > MaxVisibleQuantity) ? (LeavesQuantity - MaxVisibleQuantity) : 0; }
    //! Order visible quantity
    uint64_t VisibleQuantity() const noexcept { return std::min(LeavesQuantity, MaxVisibleQuantity); }

    //! Market order slippage
    /*!
        Slippage is useful to protect market order from executions at prices
        which are too far from the best price. If the slippage is provided
        for market order its execution will be stopped when the price run
        out of the market for the given slippage value. Zero slippage will
        allow to execute market order only at the best price, non executed
        part of the market order will be canceled.

        Supported only for market and stop orders!
    */
    uint64_t Slippage;

    //! Order trailing distance to market
    /*!
        Positive value represents absolute distance from the market.
        Negative value represents percentage distance from the market
        with 0.01% precision (-1 means 0.01, -10000 means 100%).

        Supported only for trailing stop orders!
    */
    int64_t TrailingDistance;
    //! Order trailing step
    /*!
        Positive value represents absolute step from the market.
        Negative value represents percentage step from the market
        with 0.01% precision (-1 means 0.01%, -10000 means 100%).

        Supported only for trailing stop orders!
    */
    int64_t TrailingStep;

    Order() noexcept = default;
    Order(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t stop_price, uint64_t quantity,
        OrderTimeInForce tif = OrderTimeInForce::GTC,
        uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max(),
        uint64_t slippage = std::numeric_limits<uint64_t>::max(),
        int64_t trailing_distance = 0,
        int64_t trailing_step = 0) noexcept;
    Order(const Order&) noexcept = default;
    Order(Order&&) noexcept = default;
    ~Order() noexcept = default;

    Order& operator=(const Order&) noexcept = default;
    Order& operator=(Order&&) noexcept = default;

    template <class TOutputStream>
    friend TOutputStream& operator<<(TOutputStream& stream, const Order& order);

    //! Is the market order?
    bool IsMarket() const noexcept { return Type == OrderType::MARKET; }
    //! Is the limit order?
    bool IsLimit() const noexcept { return Type == OrderType::LIMIT; }
    //! Is the stop order?
    bool IsStop() const noexcept { return Type == OrderType::STOP; }
    //! Is the stop-limit order?
    bool IsStopLimit() const noexcept { return Type == OrderType::STOP_LIMIT; }
    //! Is the trailing stop order?
    bool IsTrailingStop() const noexcept { return Type == OrderType::TRAILING_STOP; }
    //! Is the trailing stop-limit order?
    bool IsTrailingStopLimit() const noexcept { return Type == OrderType::TRAILING_STOP_LIMIT; }

    //! Is the order with buy side?
    bool IsBuy() const noexcept { return Side == OrderSide::BUY; }
    //! Is the order with sell side?
    bool IsSell() const noexcept { return Side == OrderSide::SELL; }

    //! Is the 'Good-Till-Cancelled' order?
    bool IsGTC() const noexcept { return TimeInForce == OrderTimeInForce::GTC; }
    //! Is the 'Immediate-Or-Cancel' order?
    bool IsIOC() const noexcept { return TimeInForce == OrderTimeInForce::IOC; }
    //! Is the 'Fill-Or-Kill' order?
    bool IsFOK() const noexcept { return TimeInForce == OrderTimeInForce::FOK; }
    //! Is the 'All-Or-None' order?
    bool IsAON() const noexcept { return TimeInForce == OrderTimeInForce::AON; }

    //! Is the 'Hidden' order?
    bool IsHidden() const noexcept { return MaxVisibleQuantity == 0; }
    //! Is the 'Iceberg' order?
    bool IsIceberg() const noexcept { return MaxVisibleQuantity < std::numeric_limits<uint64_t>::max(); }

    //! Is the order have slippage?
    bool IsSlippage() const noexcept { return Slippage < std::numeric_limits<uint64_t>::max(); }

    //! Validate order parameters
    ErrorCode Validate() const noexcept;

    //! Prepare a new market order
    static Order Market(uint64_t id, uint32_t symbol, OrderSide side, uint64_t quantity, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new buy market order
    static Order BuyMarket(uint64_t id, uint32_t symbol, uint64_t quantity, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new sell market order
    static Order SellMarket(uint64_t id, uint32_t symbol, uint64_t quantity, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;

    //! Prepare a new limit order
    static Order Limit(uint64_t id, uint32_t symbol, OrderSide side, uint64_t price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new buy limit order
    static Order BuyLimit(uint64_t id, uint32_t symbol, uint64_t price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new sell limit order
    static Order SellLimit(uint64_t id, uint32_t symbol, uint64_t price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;

    //! Prepare a new stop order
    static Order Stop(uint64_t id, uint32_t symbol, OrderSide side, uint64_t stop_price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new buy stop order
    static Order BuyStop(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new sell stop order
    static Order SellStop(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;

    //! Prepare a new stop-limit order
    static Order StopLimit(uint64_t id, uint32_t symbol, OrderSide side, uint64_t stop_price, uint64_t price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new buy stop-limit order
    static Order BuyStopLimit(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new sell stop-limit order
    static Order SellStopLimit(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t price, uint64_t quantity, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;

    //! Prepare a new trailing stop order
    static Order TrailingStop(uint64_t id, uint32_t symbol, OrderSide side, uint64_t stop_price, uint64_t quantity, int64_t trailing_distance, int64_t trailing_step = 0, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new trailing buy stop order
    static Order TrailingBuyStop(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t quantity, int64_t trailing_distance, int64_t trailing_step = 0, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new trailing sell stop order
    static Order TrailingSellStop(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t quantity, int64_t trailing_distance, int64_t trailing_step = 0, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t slippage = std::numeric_limits<uint64_t>::max()) noexcept;

    //! Prepare a new trailing stop-limit order
    static Order TrailingStopLimit(uint64_t id, uint32_t symbol, OrderSide side, uint64_t stop_price, uint64_t price, uint64_t quantity, int64_t trailing_distance, int64_t trailing_step = 0, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new trailing buy stop-limit order
    static Order TrailingBuyStopLimit(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t price, uint64_t quantity, int64_t trailing_distance, int64_t trailing_step = 0, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;
    //! Prepare a new trailing sell stop-limit order
    static Order TrailingSellStopLimit(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t price, uint64_t quantity, int64_t trailing_distance, int64_t trailing_step = 0, OrderTimeInForce tif = OrderTimeInForce::GTC, uint64_t max_visible_quantity = std::numeric_limits<uint64_t>::max()) noexcept;
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

    OrderNode& operator=(const Order& order) noexcept;
    OrderNode& operator=(const OrderNode&) noexcept = default;
    OrderNode& operator=(OrderNode&&) noexcept = default;
};

} // namespace Matching
} // namespace CppTrader

#include "order.inl"

#endif // CPPTRADER_MATCHING_ORDER_H
