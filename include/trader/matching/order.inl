/*!
    \file order.inl
    \brief Order inline implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

inline std::ostream& operator<<(std::ostream& stream, OrderSide side)
{
    switch (side)
    {
        case OrderSide::BUY:
            return stream << "BUY";
        case OrderSide::SELL:
            return stream << "SELL";
        default:
            return stream << "<\?\?\?>";
    }
}

inline std::ostream& operator<<(std::ostream& stream, OrderType type)
{
    switch (type)
    {
        case OrderType::MARKET:
            return stream << "MARKET";
        case OrderType::LIMIT:
            return stream << "LIMIT";
        case OrderType::STOP:
            return stream << "STOP";
        case OrderType::STOPLIMIT:
            return stream << "STOPLIMIT";
        default:
            return stream << "<\?\?\?>";
    }
}

inline std::ostream& operator<<(std::ostream& stream, OrderTimeInForce tif)
{
    switch (tif)
    {
        case OrderTimeInForce::GTC:
            return stream << "GTC";
        case OrderTimeInForce::IOC:
            return stream << "IOC";
        case OrderTimeInForce::FOK:
            return stream << "FOK";
        case OrderTimeInForce::AON:
            return stream << "AON";
        default:
            return stream << "<\?\?\?>";
    }
}

inline Order::Order(uint64_t id, uint32_t symbol, OrderType type, OrderSide side, uint64_t price, uint64_t stop_price, uint64_t quantity, OrderTimeInForce tif, uint64_t max_visible_quantity, uint64_t slippage) noexcept
    : Id(id),
      SymbolId(symbol),
      Type(type),
      Side(side),
      Price(price),
      StopPrice(stop_price),
      Quantity(quantity),
      TimeInForce(tif),
      MaxVisibleQuantity(max_visible_quantity),
      Slippage(slippage)
{
}

inline std::ostream& operator<<(std::ostream& stream, const Order& order)
{
    stream << "Order(Id=" << order.Id
        << "; SymbolId=" << order.SymbolId
        << "; Type=" << order.Type
        << "; Side=" << order.Side
        << "; Price=" << order.Price
        << "; Quantity=" << order.Quantity
        << "; " << order.TimeInForce;
    if (order.IsStop() || order.IsStopLimit())
        stream << "; StopPrice=" << order.StopPrice;
    if (order.IsHidden() || order.IsIceberg())
        stream << "; MaxVisibleQuantity=" << order.MaxVisibleQuantity;
    if (order.IsSlippage())
        stream << "; Slippage=" << order.Slippage;
    return stream << ")";
}

inline Order Order::Market(uint64_t id, uint32_t symbol, OrderSide side, uint64_t quantity, uint64_t slippage) noexcept
{
    return Order(id, symbol, OrderType::MARKET, side, 0, 0, quantity, OrderTimeInForce::IOC, std::numeric_limits<uint64_t>::max(), slippage);
}

inline Order Order::BuyMarket(uint64_t id, uint32_t symbol, uint64_t quantity, uint64_t slippage) noexcept
{
    return Order(id, symbol, OrderType::MARKET, OrderSide::BUY, 0, 0, quantity, OrderTimeInForce::IOC, std::numeric_limits<uint64_t>::max(), slippage);
}

inline Order Order::SellMarket(uint64_t id, uint32_t symbol, uint64_t quantity, uint64_t slippage) noexcept
{
    return Order(id, symbol, OrderType::MARKET, OrderSide::SELL, 0, 0, quantity, OrderTimeInForce::IOC, std::numeric_limits<uint64_t>::max(), slippage);
}

inline Order Order::Limit(uint64_t id, uint32_t symbol, OrderSide side, uint64_t price, uint64_t quantity, OrderTimeInForce tif, uint64_t max_visible_quantity) noexcept
{
    return Order(id, symbol, OrderType::LIMIT, side, price, 0, quantity, tif, max_visible_quantity, std::numeric_limits<uint64_t>::max());
}

inline Order Order::BuyLimit(uint64_t id, uint32_t symbol, uint64_t price, uint64_t quantity, OrderTimeInForce tif, uint64_t max_visible_quantity) noexcept
{
    return Order(id, symbol, OrderType::LIMIT, OrderSide::BUY, price, 0, quantity, tif, max_visible_quantity, std::numeric_limits<uint64_t>::max());
}

inline Order Order::SellLimit(uint64_t id, uint32_t symbol, uint64_t price, uint64_t quantity, OrderTimeInForce tif, uint64_t max_visible_quantity) noexcept
{
    return Order(id, symbol, OrderType::LIMIT, OrderSide::SELL, price, 0, quantity, tif, max_visible_quantity, std::numeric_limits<uint64_t>::max());
}

inline Order Order::Stop(uint64_t id, uint32_t symbol, OrderSide side, uint64_t stop_price, uint64_t quantity, OrderTimeInForce tif, uint64_t slippage) noexcept
{
    return Order(id, symbol, OrderType::STOP, side, 0, stop_price, quantity, tif, std::numeric_limits<uint64_t>::max(), slippage);
}

inline Order Order::BuyStop(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t quantity, OrderTimeInForce tif, uint64_t slippage) noexcept
{
    return Order(id, symbol, OrderType::STOP, OrderSide::BUY, 0, stop_price, quantity, tif, std::numeric_limits<uint64_t>::max(), slippage);
}

inline Order Order::SellStop(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t quantity, OrderTimeInForce tif, uint64_t slippage) noexcept
{
    return Order(id, symbol, OrderType::STOP, OrderSide::SELL, 0, stop_price, quantity, tif, std::numeric_limits<uint64_t>::max(), slippage);
}

inline Order Order::StopLimit(uint64_t id, uint32_t symbol, OrderSide side, uint64_t stop_price, uint64_t price, uint64_t quantity, OrderTimeInForce tif, uint64_t max_visible_quantity) noexcept
{
    return Order(id, symbol, OrderType::STOPLIMIT, side, price, stop_price, quantity, tif, max_visible_quantity, std::numeric_limits<uint64_t>::max());
}

inline Order Order::BuyStopLimit(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t price, uint64_t quantity, OrderTimeInForce tif, uint64_t max_visible_quantity) noexcept
{
    return Order(id, symbol, OrderType::STOPLIMIT, OrderSide::BUY, price, stop_price, quantity, tif, max_visible_quantity, std::numeric_limits<uint64_t>::max());
}

inline Order Order::SellStopLimit(uint64_t id, uint32_t symbol, uint64_t stop_price, uint64_t price, uint64_t quantity, OrderTimeInForce tif, uint64_t max_visible_quantity) noexcept
{
    return Order(id, symbol, OrderType::STOPLIMIT, OrderSide::SELL, price, stop_price, quantity, tif, max_visible_quantity, std::numeric_limits<uint64_t>::max());
}

inline OrderNode::OrderNode(const Order& order) noexcept : Order(order), Level(nullptr)
{
}

inline OrderNode& OrderNode::operator=(const Order& order) noexcept
{
    Order::operator=(order);
    Level = nullptr;
    return *this;
}

} // namespace Matching
} // namespace CppTrader
