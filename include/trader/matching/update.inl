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
        case UpdateType::ADD:
            stream << "ADD";
        case UpdateType::UPDATE:
            stream << "UPDATE";
        case UpdateType::DELETE:
            stream << "DELETE";
        default:
            stream << "<unknown>";
    }
    return stream;
}

} // namespace Matching
} // namespace CppTrader
