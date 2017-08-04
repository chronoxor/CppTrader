/*!
    \file order.inl
    \brief Order domain model inline implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

namespace CppTrader {

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
        case OrderType::LIMIT:
            return stream << "LIMIT";
        default:
            return stream << "<\?\?\?>";
    }
}

inline std::ostream& operator<<(std::ostream& stream, const Order& order)
{
    return stream << "Order(Id=" << order.Id
        << "; SymbolId=" << order.SymbolId
        << "; Type=" << order.Type
        << "; Side=" << order.Side
        << "; Price=" << order.Price
        << "; Quantity=" << order.Quantity
        << ")";
}

} // namespace CppTrader
