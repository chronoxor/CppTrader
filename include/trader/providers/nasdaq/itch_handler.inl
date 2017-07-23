/*!
    \file itch_handler.inl
    \brief NASDAQ ITCH handler inline implementation
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/

namespace CppTrader {
namespace ITCH {

template <size_t N>
inline size_t ITCHHandler::ReadString(const void* buffer, char (&str)[N])
{
    std::memcpy(str, buffer, N);

    return N;
}

inline size_t ITCHHandler::ReadTimestamp(const void* buffer, uint64_t& value)
{
    if (CppCommon::Endian::IsBigEndian())
    {
        ((uint8_t*)&value)[0] = 0;
        ((uint8_t*)&value)[1] = 0;
        ((uint8_t*)&value)[2] = 0;
        ((uint8_t*)&value)[3] = 0;
        ((uint8_t*)&value)[4] = 0;
        ((uint8_t*)&value)[5] = ((const uint8_t*)buffer)[0];
        ((uint8_t*)&value)[6] = ((const uint8_t*)buffer)[1];
        ((uint8_t*)&value)[7] = ((const uint8_t*)buffer)[2];
    }
    else
    {
        ((uint8_t*)&value)[0] = ((const uint8_t*)buffer)[2];
        ((uint8_t*)&value)[1] = ((const uint8_t*)buffer)[1];
        ((uint8_t*)&value)[2] = ((const uint8_t*)buffer)[0];
        ((uint8_t*)&value)[3] = 0;
        ((uint8_t*)&value)[4] = 0;
        ((uint8_t*)&value)[5] = 0;
        ((uint8_t*)&value)[6] = 0;
        ((uint8_t*)&value)[7] = 0;

    }

    return 6;
}

} // namespace ITCH
} // namespace CppTrader
