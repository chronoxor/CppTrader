/*!
    \file level.inl
    \brief Price level inline implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

namespace CppTrader {

inline std::ostream& operator<<(std::ostream& stream, LevelSide side)
{
    switch (side)
    {
        case LevelSide::BID:
            return stream << "BID";
        case LevelSide::ASK:
            return stream << "ASK";
        default:
            return stream << "<\?\?\?>";
    }
}

inline Level::Level(uint64_t price) noexcept
{
    Price = price;
    Volume = 0;
}

inline std::ostream& operator<<(std::ostream& stream, const Level& level)
{
    return stream << "Level(Price=" << level.Price
        << "; Volume=" << level.Volume
        << "; Orders=" << level.Orders.size()
        << ")";
}

} // namespace CppTrader
