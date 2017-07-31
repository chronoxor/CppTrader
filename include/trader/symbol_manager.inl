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

inline bool SymbolManager::RegisterSymbol(const Symbol& symbol)
{
    // Resize the symbol container
    if (_symbols.size() <= symbol.Id)
        _symbols.resize(symbol.Id + 1, nullptr);

    // Check if the symbol with a given Id is already registered
    if (_symbols[symbol.Id] != nullptr)
        return false;

    // Register symbol
    Symbol* result = _pool.Create(symbol);
    _symbols[result->Id] = result;
    _symbols_by_name[FastHash::Parse(result->Name)] = result;
    ++_size;
    return true;
}

inline bool SymbolManager::UnregisterSymbol(uint32_t id)
{
    // Check if the symbol with a given Id is registered before
    if ((_symbols.size() <= id) || (_symbols[id] == nullptr))
        return false;

    // Unregister symbol
    Symbol* result = _symbols[id];
    _symbols[id] = nullptr;
    _symbols_by_name.erase(FastHash::Parse(result->Name));
    _pool.Release(result);
    --_size;
    return true;
}

} // namespace CppTrader
