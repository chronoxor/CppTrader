/*!
    \file market_manager.inl
    \brief Market manager inline implementation
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

namespace CppTrader {

inline const OrderBook* MarketManager::GetOrderBook(uint32_t symbol) const noexcept
{
    return ((symbol < _order_book.size()) ? _order_book[symbol] : nullptr);
}

} // namespace CppTrader
