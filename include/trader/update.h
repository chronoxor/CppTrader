/*!
    \file update.h
    \brief Update definition
    \author Ivan Shynkarenka
    \date 09.08.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_UPDATE_H
#define CPPTRADER_UPDATE_H

#include <cstdint>

namespace CppTrader {

//! Update type
enum class UpdateType : uint8_t
{
    NONE,
    ADD,
    UPDATE,
    DELETE
};
std::ostream& operator<<(std::ostream& stream, UpdateType type);

} // namespace CppTrader

#include "update.inl"

#endif // CPPTRADER_UPDATE_H
