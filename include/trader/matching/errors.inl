/*!
    \file errors.inl
    \brief Errors inline implementation
    \author Ivan Shynkarenka
    \date 16.08.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

inline std::ostream& operator<<(std::ostream& stream, ErrorCode error)
{
    switch (error)
    {
        case ErrorCode::OK:
            return stream << "OK";
        case ErrorCode::SYMBOL_DUPLICATE:
            return stream << "SYMBOL_DUPLICATE";
        case ErrorCode::SYMBOL_NOT_FOUND:
            return stream << "SYMBOL_NOT_FOUND";
        case ErrorCode::ORDER_BOOK_DUPLICATE:
            return stream << "ORDER_BOOK_DUPLICATE";
        case ErrorCode::ORDER_BOOK_NOT_FOUND:
            return stream << "ORDER_BOOK_NOT_FOUND";
        case ErrorCode::ORDER_DUPLICATE:
            return stream << "ORDER_DUPLICATE";
        case ErrorCode::ORDER_NOT_FOUND:
            return stream << "ORDER_NOT_FOUND";
        case ErrorCode::ORDER_ID_INVALID:
            return stream << "ORDER_ID_INVALID";
        case ErrorCode::ORDER_TYPE_INVALID:
            return stream << "ORDER_TYPE_INVALID";
        case ErrorCode::ORDER_PARAMETER_INVALID:
            return stream << "ORDER_PARAMETER_INVALID";
        case ErrorCode::ORDER_QUANTITY_INVALID:
            return stream << "ORDER_QUANTITY_INVALID";
        default:
            return stream << "<unknown>";
    }
}

} // namespace Matching
} // namespace CppTrader
