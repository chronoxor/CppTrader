/*!
    \file update.inl
    \brief Update inline implementation
    \author Ivan Shynkarenka
    \date 09.08.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

inline std::ostream& operator<<(std::ostream& stream, UpdateType type)
{
    switch (type)
    {
        case UpdateType::NONE:
            return stream << "NONE";
        case UpdateType::ADD:
            return stream << "ADD";
        case UpdateType::UPDATE:
            return stream << "UPDATE";
        case UpdateType::DELETE:
            return stream << "DELETE";
        default:
            return stream << "<\?\?\?>";
    }
}

} // namespace Matching
} // namespace CppTrader
