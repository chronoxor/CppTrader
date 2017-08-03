/*!
    \file symbol_manager.cpp
    \brief Symbol manager implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#include "trader/symbol_manager.h"

namespace CppTrader {

bool SymbolManager::RegisterSymbol(const Symbol& symbol)
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

bool SymbolManager::UnregisterSymbol(uint32_t id)
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
