/*!
    \file update.inl
    \brief Update inline implementation
    \author Ivan Shynkarenka
    \date 09.08.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, UpdateType type)
{
    switch (type)
    {
        case UpdateType::NONE:
            stream << "NONE";
            break;
        case UpdateType::ADD:
            stream << "ADD";
            break;
        case UpdateType::UPDATE:
            stream << "UPDATE";
            break;
        case UpdateType::DELETE:
            stream << "DELETE";
            break;
        default:
            stream << "<unknown>";
            break;
    }
    return stream;
}

} // namespace Matching
} // namespace CppTrader
