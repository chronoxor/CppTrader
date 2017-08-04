/*!
    \file symbol.h
    \brief Symbol domain model definition
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_DOMAIN_SYMBOL_H
#define CPPTRADER_DOMAIN_SYMBOL_H

#include "utility/stream.h"

#include <cstdint>
#include <cstring>

namespace CppTrader {

//! Symbol model
struct Symbol
{
    //! Symbol Id
    uint32_t Id;
    //! Symbol name
    char Name[8];

    Symbol(uint32_t id, const char name[8]) noexcept
    {
        Id = id;
        std::strncpy(Name, name, sizeof(Name));
    }
    Symbol(const Symbol&) noexcept = default;
    Symbol(Symbol&&) noexcept = default;
    ~Symbol() noexcept = default;

    Symbol& operator=(const Symbol&) noexcept = default;
    Symbol& operator=(Symbol&&) noexcept = default;

    friend std::ostream& operator<<(std::ostream& stream, const Symbol& symbol);
};

} // namespace CppTrader

#include "symbol.inl"

#endif // CPPTRADER_DOMAIN_SYMBOL_H
