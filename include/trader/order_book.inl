/*!
    \file order_book.inl
    \brief Order book inline implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

namespace CppTrader {

inline OrderBook::OrderBook(const Symbol& symbol)
    : _symbol(symbol),
      _default_manager(),
      _pool_manager(_default_manager, 1024),
      _pool(_pool_manager)
{
}

inline std::ostream& operator<<(std::ostream& stream, const OrderBook& order_book)
{
    return stream << "OrderBook(Symbol=" << order_book._symbol
        << "; Bids=" << order_book._bids.size()
        << "; Asks=" << order_book._asks.size()
        << ")";
}

} // namespace CppTrader
