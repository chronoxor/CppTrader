/*!
    \file fast_hash.h
    \brief Fast hash helper definition
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_FAST_HASH_H
#define CPPTRADER_FAST_HASH_H

#include <cstddef>
#include <cstdint>

namespace CppTrader {

//! Fast hash helper
/*!
    Fast hash helper is used to calculate 64-bit hash in a fast way.

    Thread-safe.
*/
class FastHash
{
public:
    FastHash() = default;
    FastHash(const FastHash&) = default;
    FastHash(FastHash&&) noexcept = default;
    ~FastHash() = default;

    FastHash& operator=(const FastHash&) = default;
    FastHash& operator=(FastHash&&) noexcept = default;

    //! Calculate hash value
    size_t operator()(uint64_t value) const noexcept;

    //! Parse fixed size string value and return its 64-bit integer equivalent
    /*!
        \param str - Fixed size string (8 bytes length)
        \return 64-bit integer value
    */
    static uint64_t Parse(const char str[8]) noexcept;
};

} // namespace CppTrader

#include "fast_hash.inl"

#endif // CPPTRADER_FAST_HASH_H
