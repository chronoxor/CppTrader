/*!
    \file errors.inl
    \brief Errors inline implementation
    \author Ivan Shynkarenka
    \date 16.08.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace Matching {

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, ErrorCode error)
{
    switch (error)
    {
        case ErrorCode::OK:
            stream << "OK";
        case ErrorCode::SYMBOL_DUPLICATE:
            stream << "SYMBOL_DUPLICATE";
        case ErrorCode::SYMBOL_NOT_FOUND:
            stream << "SYMBOL_NOT_FOUND";
        case ErrorCode::ORDER_BOOK_DUPLICATE:
            stream << "ORDER_BOOK_DUPLICATE";
        case ErrorCode::ORDER_BOOK_NOT_FOUND:
            stream << "ORDER_BOOK_NOT_FOUND";
        case ErrorCode::ORDER_DUPLICATE:
            stream << "ORDER_DUPLICATE";
        case ErrorCode::ORDER_NOT_FOUND:
            stream << "ORDER_NOT_FOUND";
        case ErrorCode::ORDER_ID_INVALID:
            stream << "ORDER_ID_INVALID";
        case ErrorCode::ORDER_TYPE_INVALID:
            stream << "ORDER_TYPE_INVALID";
        case ErrorCode::ORDER_PARAMETER_INVALID:
            stream << "ORDER_PARAMETER_INVALID";
        case ErrorCode::ORDER_QUANTITY_INVALID:
            stream << "ORDER_QUANTITY_INVALID";
        default:
            stream << "<unknown>";
    }
    return stream;
}

} // namespace Matching
} // namespace CppTrader
