/*!
    \file market_manager.inl
    \brief Market manager inline implementation
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

namespace CppTrader {

inline MarketManager::MarketManager()
    : MarketManager(_default_handler)
{
}

inline MarketManager::MarketManager(MarketHandler& market_handler)
    : _market_handler(market_handler),
      _default_manager(),
      _pool_manager(_default_manager),
      _pool(_pool_manager)
{

}

inline const OrderBook* MarketManager::GetOrderBook(uint32_t symbol) const noexcept
{
    return ((symbol < _order_book.size()) ? _order_book[symbol] : nullptr);
}

} // namespace CppTrader
