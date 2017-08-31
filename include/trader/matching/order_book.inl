/*!
    \file order_book.inl
    \brief Order book inline implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

inline OrderBook::OrderBook(const Symbol& symbol)
    : _symbol(symbol),
      _auxiliary_memory_manager(),
      _level_memory_manager(_auxiliary_memory_manager, 1024),
      _level_pool(_level_memory_manager),
      _best_bid(nullptr),
      _best_ask(nullptr)
{
}

inline std::ostream& operator<<(std::ostream& stream, const OrderBook& order_book)
{
    return stream << "OrderBook(Symbol=" << order_book._symbol
        << "; Bids=" << order_book._bids.size()
        << "; Asks=" << order_book._asks.size()
        << ")";
}

inline const LevelNode* OrderBook::GetBid(uint64_t price) const noexcept
{
    auto it = _bids.find(LevelNode(LevelType::BID, price));
    return (it != _bids.end()) ? it.operator->() : nullptr;
}

inline const LevelNode* OrderBook::GetAsk(uint64_t price) const noexcept
{
    auto it = _asks.find(LevelNode(LevelType::ASK, price));
    return (it != _asks.end()) ? it.operator->() : nullptr;
}

inline LevelNode* OrderBook::GetNextLevel(LevelNode* level) noexcept
{
    if (level->IsBid())
    {
        Levels::reverse_iterator it(&_bids, level);
        ++it;
        return it.operator->();
    }
    else
    {
        Levels::iterator it(&_asks, level);
        ++it;
        return it.operator->();
    }
}

} // namespace Matching
} // namespace CppTrader
