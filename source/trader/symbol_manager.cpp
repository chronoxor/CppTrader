/*!
    \file symbol_manager.cpp
    \brief Symbol manager implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#include "trader/symbol_manager.h"

#include "errors/exceptions.h"
#include "string/format.h"

namespace CppTrader {

SymbolManager::~SymbolManager()
{
    for (auto symbol : _symbols)
        if (symbol != nullptr)
            _pool.Release(symbol);
     _symbols.clear();
    _symbols_by_name.clear();
    _size = 0;
}

Symbol* SymbolManager::AddSymbol(const Symbol& symbol)
{
    // Resize the symbol container
    if (_symbols.size() <= symbol.Id)
        _symbols.resize(symbol.Id + 1, nullptr);

    assert((_symbols[symbol.Id] == nullptr) && "Duplicate symbol detected!");
    if (_symbols[symbol.Id] != nullptr)
        throwex CppCommon::RuntimeException("Duplicate symbol detected! Symbol Id = {}"_format(symbol.Id));

    // Add the symbol
    Symbol* new_symbol = _pool.Create(symbol);
    _symbols[symbol.Id] = new_symbol;
    _symbols_by_name[FastHash::Parse(symbol.Name)] = new_symbol;

    ++_size;

    return new_symbol;
}

void SymbolManager::DeleteSymbol(uint32_t id)
{
    assert(((id < _symbols.size()) && (_symbols[id] != nullptr)) && "Symbol not found!");
    if ((_symbols.size() <= id) || (_symbols[id] == nullptr))
        throwex CppCommon::RuntimeException("Symbol not found! Symbol Id = {}"_format(id));

    // Delete the symbol
    Symbol* symbol = _symbols[id];
    _symbols[id] = nullptr;
    _symbols_by_name.erase(FastHash::Parse(symbol->Name));
    _pool.Release(symbol);

    --_size;
}

} // namespace CppTrader
