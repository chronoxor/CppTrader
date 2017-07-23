/*!
    \file itch_handler.cpp
    \brief NASDAQ ITCH handler implementation
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/

#include "trader/providers/nasdaq/itch_handler.h"

#include "utility/endian.h"

#include <cassert>

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
    uint8_t* data = (uint8_t*)buffer;

    while (index < size)
    {
        size_t remaining = size - index;

        // Collect message size in cache
        if (((_cache.size() == 0) && (remaining < 3)) || (_cache.size() == 1))
        {
            _cache.push_back(data[index++]);
            continue;
        }

        // Read a new message size
        if (_size == 0)
        {
            uint16_t message_size;
            CppCommon::Endian::ReadBigEndian(_cache.empty() ? &data[index++] : _cache.data(), message_size);
            _size = message_size;
            _cache.clear();
        }

        // Read a new message
        if (_size > 0)
        {
            // Complete or place the message in the cache
            if (!_cache.empty())
            {
                size_t tail = _size - _cache.size();
                if (tail > remaining)
                    tail = remaining;
                _cache.insert(_cache.end(), &data[index], &data[tail]);
                index += tail;
                if (_cache.size() < _size)
                    continue;
            }
            else if (_size > remaining)
            {
                _cache.reserve(_size);
                _cache.insert(_cache.end(), &data[index], &data[remaining]);
                index += remaining;
                continue;
            }

            // Process the current message
            if (!ProcessMessage(_cache.empty() ? &data[index] : _cache.data(), _size))
                return false;

            index += _size;

            // Reset to the next message
            Reset();
        }
    }

    return true;
}

bool ITCHHandler::ProcessMessage(void* buffer, size_t size)
{
    // Message is empty
    if (size == 0)
        return false;

    uint8_t* data = (uint8_t*)buffer;
    char type = *data++;

    switch (type)
    {
        case 'S':
            return ProcessSystemEventMessage(data, size - 1);
        default:
            return ProcessUnknownMessage(type);
    }
}

bool ITCHHandler::ProcessSystemEventMessage(void* buffer, size_t size)
{
    assert((size == 6) && "Invalid size of the ITCH message type 'S'");
    if (size != 6)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    SystemEventMessage message;
    CppCommon::Endian::ReadBigEndian(data, message.Timestamp);
    data += 4;
    message.EventCode = (SystemEventCodes)*data;
    data += 1;

    return HandleMessage(message);
}

bool ITCHHandler::ProcessUnknownMessage(uint8_t type)
{
    UnknownMessage message;
    message.Type = type;

    return HandleMessage(message);
}

void ITCHHandler::Reset()
{
    _size = 0;
    _cache.clear();
}

} // namespace ITCH
} // namespace CppTrader
