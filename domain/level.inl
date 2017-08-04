/*!
    \file level.inl
    \brief Price level domain model inline implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

namespace CppTrader {

inline std::ostream& operator<<(std::ostream& stream, const Level& level)
{
    return stream << "Level(Price=" << level.Price
        << "; Volume=" << level.Volume
        << "; Orders=" << level.Orders.size()
        << ")";
}

} // namespace CppTrader
