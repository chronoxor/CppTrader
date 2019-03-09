/*!
    \file level.inl
    \brief Price level inline implementation
    \author Ivan Shynkarenka
    \date 02.08.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, LevelType type)
{
    switch (type)
    {
        case LevelType::BID:
            stream << "BID";
            break;
        case LevelType::ASK:
            stream << "ASK";
            break;
        default:
            stream << "<unknown>";
            break;
    }
    return stream;
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

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const Level& level)
{
    stream << "Level(Type=" << level.Type
        << "; Price=" << level.Price
        << "; TotalVolume=" << level.TotalVolume
        << "; HiddenVolume=" << level.HiddenVolume
        << "; VisibleVolume=" << level.VisibleVolume
        << "; Orders=" << level.Orders
        << ")";
    return stream;
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

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const LevelUpdate& update)
{
    stream << "LevelUpdate(Type=" << update.Type
        << "; Update=" << update.Update
        << "; Top=" << update.Top
        << ")";
    return stream;
}

} // namespace Matching
} // namespace CppTrader
