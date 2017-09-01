/*!
    \file level.inl
    \brief Price level inline implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

inline std::ostream& operator<<(std::ostream& stream, LevelType type)
{
    switch (type)
    {
        case LevelType::BID:
            return stream << "BID";
        case LevelType::ASK:
            return stream << "ASK";
        default:
            return stream << "<\?\?\?>";
    }
}

inline Level::Level(LevelType type, uint64_t price) noexcept
    : Type(type),
      Price(price),
      TotalVolume(0),
      HiddenVolume(0),
      VisibleVolume(0),
      Orders(0)
{
}

inline std::ostream& operator<<(std::ostream& stream, const Level& level)
{
    return stream << "Level(Type=" << level.Type
        << "; Price=" << level.Price
        << "; TotalVolume=" << level.TotalVolume
        << "; HiddenVolume=" << level.HiddenVolume
        << "; VisibleVolume=" << level.VisibleVolume
        << "; Orders=" << level.Orders
        << ")";
}

inline LevelNode::LevelNode(LevelType type, uint64_t price) noexcept
    : Level(type, price)
{
}

inline LevelNode::LevelNode(const Level& level) noexcept : Level(level)
{
}

inline LevelNode& LevelNode::operator=(const Level& level) noexcept
{
    Level::operator=(level);
    OrderList.clear();
    return *this;
}

inline LevelUpdate::LevelUpdate(UpdateType type, const Level& update, bool top) noexcept
    : Type(type),
      Update(update),
      Top(top)
{
}

inline std::ostream& operator<<(std::ostream& stream, const LevelUpdate& update)
{
    return stream << "LevelUpdate(Type=" << update.Type
        << "; Update=" << update.Update
        << "; Top=" << update.Top
        << ")";
}

} // namespace Matching
} // namespace CppTrader
