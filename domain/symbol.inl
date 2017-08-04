/*!
    \file symbol.inl
    \brief Symbol domain model inline implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

namespace CppTrader {

inline std::ostream& operator<<(std::ostream& stream, const Symbol& symbol)
{
    return stream << "Symbol(Id=" << symbol.Id
        << "; Name=" << CppCommon::WriteString(symbol.Name)
        << ")";
}

} // namespace CppTrader
