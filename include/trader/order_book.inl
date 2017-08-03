/*!
    \file order_book.inl
    \brief Order book inline implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

namespace CppTrader {

inline OrderBook::OrderBook()
    : _default_manager(),
      _pool_manager(_default_manager, 1024),
      _pool(_pool_manager)
{
}

} // namespace CppTrader
