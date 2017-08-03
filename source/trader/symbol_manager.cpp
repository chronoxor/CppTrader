/*!
    \file symbol_manager.cpp
    \brief Symbol manager implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#include "trader/symbol_manager.h"

namespace CppTrader {

bool SymbolManager::AddSymbol(uint32_t id, const char name[8])
{
    // Resize the symbol container
    if (_symbols.size() <= id)
        _symbols.resize(id + 1, nullptr);

    // Check if the symbol with a given Id is already added
    if (_symbols[id] != nullptr)
        return false;

    // Add the symbol
    Symbol* result = _pool.Create(id, name);
    _symbols[id] = result;
    _symbols_by_name[FastHash::Parse(result->Name)] = result;

    ++_size;

    return true;
}

bool SymbolManager::RemoveSymbol(uint32_t id)
{
    // Check if the symbol with a given Id is added before
    if ((_symbols.size() <= id) || (_symbols[id] == nullptr))
        return false;

    // Remove the symbol
    Symbol* result = _symbols[id];
    _symbols[id] = nullptr;
    _symbols_by_name.erase(FastHash::Parse(result->Name));
    _pool.Release(result);

    --_size;

    return true;
}

} // namespace CppTrader
