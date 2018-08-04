/*!
    \file symbol.inl
    \brief Symbol inline implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

inline Symbol::Symbol(uint32_t id, const char name[8]) noexcept
    : Id(id)
{
    std::memcpy(Name, name, sizeof(Name));
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const Symbol& symbol)
{
    stream << "Symbol(Id=" << symbol.Id
        << "; Name=" << CppCommon::WriteString(symbol.Name)
        << ")";
    return stream;
}

} // namespace Matching
} // namespace CppTrader
