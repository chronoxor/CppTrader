/*!
    \file itch_handler.cpp
    \brief NASDAQ ITCH handler implementation
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/

#include "trader/providers/nasdaq/itch_handler.h"

namespace CppTrader {
namespace ITCH {

std::ostream& operator<<(std::ostream& stream, SystemEventCodes e)
{
    switch (e)
    {
        case SystemEventCodes::START_OF_MESSAGES:
            stream << "START_OF_MESSAGES ('O')";
            break;
        case SystemEventCodes::START_OF_SYSTEM_HOURS:
            stream << "START_OF_SYSTEM_HOURS ('S')";
            break;
        case SystemEventCodes::START_OF_MARKET_HOURS:
            stream << "START_OF_MARKET_HOURS ('Q')";
            break;
        case SystemEventCodes::END_OF_SYSTEM_HOURS:
            stream << "END_OF_SYSTEM_HOURS ('E')";
            break;
        case SystemEventCodes::END_OF_MESSAGES:
            stream << "END_OF_MESSAGES ('C')";
            break;
        case SystemEventCodes::EMERGENCY_MARKET_CONDITION_HALT:
            stream << "EMERGENCY_MARKET_CONDITION_HALT ('A')";
            break;
        case SystemEventCodes::EMERGENCY_MARKET_CONDITION_QUOTE_ONLY_PERIOD:
            stream << "EMERGENCY_MARKET_CONDITION_QUOTE_ONLY_PERIOD ('R')";
            break;
        case SystemEventCodes::EMERGENCY_MARKET_CONDITION_RESUMPTION:
            stream << "EMERGENCY_MARKET_CONDITION_RESUMPTION ('B')";
            break;
        default:
            stream << "<\?\?\?>";
            break;
    }

    return stream;
}

bool ITCHHandler::Process(void* buffer, size_t size)
{
    size_t index = 0;
    uint8_t data = (uint8_t*)buffer;

    while (index < size)
    {
        size_t remaining = size - index;

        // Collect message size in cache
        if ((_cache.size() < 2) && (remaining < 3))
        {
            _cache.push_back(data[index++]);
            continue;
        }

        size_t message_size;
    }
}

void ITCHHandler::Reset()
{
    _size = 0;
    _cache.clear();
}

} // namespace ITCH
} // namespace CppTrader
