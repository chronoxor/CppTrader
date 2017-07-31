/*!
    \file symbol_manager.h
    \brief Symbol manager definition
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_SYMBOL_MANAGER_H
#define CPPTRADER_SYMBOL_MANAGER_H

#include "../../domain/symbol.h"

#include "fast_hash.h"

#include "containers/hashmap.h"
#include "memory/allocator_pool.h"

#include <vector>

namespace CppTrader {

//! Symbol manager
/*!
    Symbol manager is used to register and manage symbols and symbol groups.

    Not thread-safe.
*/
class SymbolManager
{
public:
    SymbolManager();
    SymbolManager(const SymbolManager&) = delete;
    SymbolManager(SymbolManager&&) noexcept = default;
    ~SymbolManager() = default;

    SymbolManager& operator=(const SymbolManager&) = delete;
    SymbolManager& operator=(SymbolManager&&) noexcept = default;

    //! Check if the symbol manager is not empty
    explicit operator bool() const noexcept { return !empty(); }

    //! Get the symbol with the given Id
    const Symbol* operator[](uint32_t id) { return GetSymbol(id); }

    //! Is the symbol manager empty?
    bool empty() const noexcept { return _size == 0; }

    //! Get the symbol manager size
    size_t size() const noexcept { return _size; }

    //! Get the symbol with the given Id
    /*!
        \param id - Symbol Id
        \return Pointer to the symobl with the given Id or nullptr
    */
    const Symbol* GetSymbol(uint32_t id) const noexcept;
    //! Get the symbol with the given name
    /*!
        \param name - Symbol name
        \return Pointer to the symobl with the given name or nullptr
    */
    const Symbol* GetSymbolByName(const char name[8]) const noexcept;

    //! Register a new symbol
    /*!
        \param symbol - Symbol to register
        \return 'true' if the symbol was successfully registered, 'false' if the symbol failed to register
    */
    bool RegisterSymbol(const Symbol& symbol);
    //! Unregister the symbol with the given Id
    /*!
        \param id - Symbol Id
        \return 'true' if the symbol was successfully unregistered, 'false' if the symbol failed to unregister
    */
    bool UnregisterSymbol(uint32_t id);

private:
    CppCommon::DefaultMemoryManager _default_manager;
    CppCommon::PoolMemoryManager<CppCommon::DefaultMemoryManager> _pool_manager;
    CppCommon::PoolAllocator<Symbol, CppCommon::DefaultMemoryManager> _pool;
    std::vector<Symbol*> _symbols;
    CppCommon::HashMap<uint64_t, Symbol*, FastHash> _symbols_by_name;
    size_t _size;
};

} // namespace CppTrader

#include "symbol_manager.inl"

#endif // CPPTRADER_SYMBOL_MANAGER_H
