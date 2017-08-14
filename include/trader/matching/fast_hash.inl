/*!
    \file fast_hash.inl
    \brief Fast hash helper inline implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

inline size_t FastHash::operator()(uint64_t value) const noexcept
{
    value ^= value >> 33;
    value *= 0xFF51AFD7ED558CCD;
    value ^= value >> 33;
    value *= 0xC4CEB9FE1A85EC53;
    value ^= value >> 33;
    return value;
}

inline uint64_t FastHash::Parse(const char str[8]) noexcept
{
    uint64_t value = 0;
    value |= ((uint64_t)str[0]);
    value |= ((uint64_t)str[1]) << 8;
    value |= ((uint64_t)str[2]) << 16;
    value |= ((uint64_t)str[3]) << 24;
    value |= ((uint64_t)str[4]) << 32;
    value |= ((uint64_t)str[5]) << 40;
    value |= ((uint64_t)str[6]) << 48;
    value |= ((uint64_t)str[7]) << 56;
    return value;
}

} // namespace Matching
} // namespace CppTrader
