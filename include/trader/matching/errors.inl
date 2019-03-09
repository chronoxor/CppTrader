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
            break;
        case ErrorCode::SYMBOL_DUPLICATE:
            stream << "SYMBOL_DUPLICATE";
            break;
        case ErrorCode::SYMBOL_NOT_FOUND:
            stream << "SYMBOL_NOT_FOUND";
            break;
        case ErrorCode::ORDER_BOOK_DUPLICATE:
            stream << "ORDER_BOOK_DUPLICATE";
            break;
        case ErrorCode::ORDER_BOOK_NOT_FOUND:
            stream << "ORDER_BOOK_NOT_FOUND";
            break;
        case ErrorCode::ORDER_DUPLICATE:
            stream << "ORDER_DUPLICATE";
            break;
        case ErrorCode::ORDER_NOT_FOUND:
            stream << "ORDER_NOT_FOUND";
            break;
        case ErrorCode::ORDER_ID_INVALID:
            stream << "ORDER_ID_INVALID";
            break;
        case ErrorCode::ORDER_TYPE_INVALID:
            stream << "ORDER_TYPE_INVALID";
            break;
        case ErrorCode::ORDER_PARAMETER_INVALID:
            stream << "ORDER_PARAMETER_INVALID";
            break;
        case ErrorCode::ORDER_QUANTITY_INVALID:
            stream << "ORDER_QUANTITY_INVALID";
            break;
        default:
            stream << "<unknown>";
            break;
    }
    return stream;
}

} // namespace Matching
} // namespace CppTrader
