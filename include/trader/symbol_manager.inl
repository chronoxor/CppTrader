/*!
    \file symbol_manager.inl
    \brief Symbol manager inline implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

namespace CppTrader {

inline SymbolManager::SymbolManager()
    : _default_manager(),
      _pool_manager(_default_manager),
      _pool(_pool_manager),
      _symbols(),
      _symbols_by_name(16384, 0),
      _size(0)
{
}

inline const Symbol* SymbolManager::GetSymbol(uint32_t id) const noexcept
{
    return ((id < _symbols.size()) ? _symbols[id] : nullptr);
}

inline const Symbol* SymbolManager::GetSymbolByName(const char name[8]) const noexcept
{
    auto it = _symbols_by_name.find(FastHash::Parse(name));
    return ((it != _symbols_by_name.end()) ? it->second : nullptr);
}

} // namespace CppTrader
