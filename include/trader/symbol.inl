/*!
    \file symbol.inl
    \brief Symbol inline implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

namespace CppTrader {

inline Symbol::Symbol(uint32_t id, const char name[8]) noexcept
    : Id(id)
{
    std::strncpy(Name, name, sizeof(Name));
}

inline std::ostream& operator<<(std::ostream& stream, const Symbol& symbol)
{
    return stream << "Symbol(Id=" << symbol.Id
        << "; Name=" << CppCommon::WriteString(symbol.Name)
        << ")";
}

} // namespace CppTrader
