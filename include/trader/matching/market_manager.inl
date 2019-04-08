/*!
    \file market_manager.inl
    \brief Market manager inline implementation
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

inline MarketManager::MarketManager()
    : MarketManager(_default)
{
}

inline MarketManager::MarketManager(MarketHandler& market_handler)
    : _market_handler(market_handler),
      _auxiliary_memory_manager(),
      _level_memory_manager(_auxiliary_memory_manager),
      _level_pool(_level_memory_manager),
      _symbol_memory_manager(_auxiliary_memory_manager),
      _symbol_pool(_symbol_memory_manager),
      _order_book_memory_manager(_auxiliary_memory_manager),
      _order_book_pool(_order_book_memory_manager),
      _order_memory_manager(_auxiliary_memory_manager),
      _order_pool(_order_memory_manager),
      _orders(16384, 0),
      _matching(false)
{

}

inline const Symbol* MarketManager::GetSymbol(uint32_t id) const noexcept
{
    return ((id < _symbols.size()) ? _symbols[id] : nullptr);
}

inline const OrderBook* MarketManager::GetOrderBook(uint32_t id) const noexcept
{
    return ((id < _order_books.size()) ? _order_books[id] : nullptr);
}

inline const Order* MarketManager::GetOrder(uint64_t id) const noexcept
{
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        return nullptr;

    auto it = _orders.find(id);
    return ((it != _orders.end()) ? it->second : nullptr);
}

} // namespace Matching
} // namespace CppTrader
