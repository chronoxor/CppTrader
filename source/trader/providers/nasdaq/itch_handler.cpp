/*!
    \file itch_handler.cpp
    \brief NASDAQ ITCH handler implementation
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/

#include "trader/providers/nasdaq/itch_handler.h"

#include <cassert>

namespace {

class WriteChar
{
public:
    explicit WriteChar(char ch) : _ch(ch) {}

    friend std::ostream& operator<<(std::ostream& stream, const WriteChar& writer)
    {
        return stream << '\'' << writer._ch << '\'';
    }

private:
    char _ch;
};

class WriteString
{
public:
    template <size_t N>
    explicit WriteString(const char (&str)[N]) : _str(str), _size(N) {}

    friend std::ostream& operator<<(std::ostream& stream, const WriteString& writer)
    {
        stream << '"';
        stream.write(writer._str, writer._size);
        stream << '"';
        return stream;
    }

private:
    const char* _str;
    size_t _size;
};

}

namespace CppTrader {
namespace ITCH {

bool ITCHHandler::Process(void* buffer, size_t size)
{
    size_t index = 0;
    uint8_t* data = (uint8_t*)buffer;

    while (index < size)
    {
        size_t remaining = size - index;

        if (_size == 0)
        {
            // Collect message size in cache
            if (((_cache.size() == 0) && (remaining < 3)) || (_cache.size() == 1))
            {
                _cache.push_back(data[index++]);
                continue;
            }

            // Read a new message size
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

    switch (*data)
    {
        case 'S':
            return ProcessSystemEventMessage(data, size);
        case 'R':
            return ProcessStockDirectoryMessage(data, size);
        case 'H':
            return ProcessStockTradingActionMessage(data, size);
        case 'Y':
            return ProcessRegSHOMessage(data, size);
        case 'L':
            return ProcessMarketParticipantPositionMessage(data, size);
        case 'V':
            return ProcessMWCBDeclineMessage(data, size);
        case 'W':
            return ProcessMWCBStatusMessage(data, size);
        case 'K':
            return ProcessIPOQuotingMessage(data, size);
        default:
            return ProcessUnknownMessage(data, size);
    }
}

void ITCHHandler::Reset()
{
    _size = 0;
    _cache.clear();
}

bool ITCHHandler::ProcessSystemEventMessage(void* buffer, size_t size)
{
    assert((size == 12) && "Invalid size of the ITCH message type 'S'");
    if (size != 12)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    SystemEventMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    message.EventCode = *data++;

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const SystemEventMessage& message)
{
    return stream << "SystemEventMessage(Type=" << WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; EventCode=" << WriteChar(message.EventCode)
        << ")";
}

bool ITCHHandler::ProcessStockDirectoryMessage(void* buffer, size_t size)
{
    assert((size == 39) && "Invalid size of the ITCH message type 'R'");
    if (size != 39)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    StockDirectoryMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += ReadString(data, message.Stock);
    message.MarketCategory = *data++;
    message.FinancialStatusIndicator = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.RoundLotSize);
    message.RoundLotsOnly = *data++;
    message.IssueClassification = *data++;
    data += ReadString(data, message.IssueSubType);
    message.Authenticity = *data++;
    message.ShortSaleThresholdIndicator = *data++;
    message.IPOFlag = *data++;
    message.LULDReferencePriceTier = *data++;
    message.ETPFlag = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.ETPLeverageFactor);
    message.InverseIndicator = *data++;

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const StockDirectoryMessage& message)
{
    return stream << "StockDirectoryMessage(Type=" << WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << WriteString(message.Stock)
        << "; MarketCategory=" << WriteChar(message.MarketCategory)
        << "; FinancialStatusIndicator=" << WriteChar(message.FinancialStatusIndicator)
        << "; RoundLotSize=" << message.RoundLotSize
        << "; RoundLotsOnly=" << WriteChar(message.RoundLotsOnly)
        << "; IssueClassification=" << WriteChar(message.IssueClassification)
        << "; IssueSubType=" << WriteString(message.IssueSubType)
        << "; Authenticity=" << WriteChar(message.Authenticity)
        << "; ShortSaleThresholdIndicator=" << WriteChar(message.ShortSaleThresholdIndicator)
        << "; IPOFlag=" << WriteChar(message.IPOFlag)
        << "; LULDReferencePriceTier=" << WriteChar(message.LULDReferencePriceTier)
        << "; ETPFlag=" << WriteChar(message.ETPFlag)
        << "; ETPLeverageFactor=" << message.ETPLeverageFactor
        << "; InverseIndicator=" << WriteChar(message.InverseIndicator)
        << ")";
}

bool ITCHHandler::ProcessStockTradingActionMessage(void* buffer, size_t size)
{
    assert((size == 25) && "Invalid size of the ITCH message type 'H'");
    if (size != 25)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    StockTradingActionMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += ReadString(data, message.Stock);
    message.TradingState = *data++;
    message.Reserved = *data++;
    message.Reason = *data++;

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const StockTradingActionMessage& message)
{
    return stream << "StockTradingActionMessage(Type=" << WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << WriteString(message.Stock)
        << "; TradingState=" << WriteChar(message.TradingState)
        << "; Reserved=" << WriteChar(message.Reserved)
        << "; Reason=" << WriteChar(message.Reason)
        << ")";
}

bool ITCHHandler::ProcessRegSHOMessage(void* buffer, size_t size)
{
    assert((size == 20) && "Invalid size of the ITCH message type 'Y'");
    if (size != 20)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    RegSHOMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += ReadString(data, message.Stock);
    message.RegSHOAction = *data++;

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const RegSHOMessage& message)
{
    return stream << "RegSHOMessage(Type=" << WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << WriteString(message.Stock)
        << "; RegSHOAction=" << WriteChar(message.RegSHOAction)
        << ")";
}

bool ITCHHandler::ProcessMarketParticipantPositionMessage(void* buffer, size_t size)
{
    assert((size == 26) && "Invalid size of the ITCH message type 'L'");
    if (size != 26)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    MarketParticipantPositionMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += ReadString(data, message.MPID);
    data += ReadString(data, message.Stock);
    message.PrimaryMarketMaker = *data++;
    message.MarketMakerMode = *data++;
    message.MarketParticipantState = *data++;

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const MarketParticipantPositionMessage& message)
{
    return stream << "MarketParticipantPositionMessage(Type=" << WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; MPID=" << WriteString(message.MPID)
        << "; Stock=" << WriteString(message.Stock)
        << "; PrimaryMarketMaker=" << WriteChar(message.PrimaryMarketMaker)
        << "; MarketMakerMode=" << WriteChar(message.MarketMakerMode)
        << "; MarketParticipantState=" << WriteChar(message.MarketParticipantState)
        << ")";
}

bool ITCHHandler::ProcessMWCBDeclineMessage(void* buffer, size_t size)
{
    assert((size == 35) && "Invalid size of the ITCH message type 'V'");
    if (size != 35)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    MWCBDeclineMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.Level1);
    data += CppCommon::Endian::ReadBigEndian(data, message.Level2);
    data += CppCommon::Endian::ReadBigEndian(data, message.Level3);

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const MWCBDeclineMessage& message)
{
    return stream << "MWCBDeclineMessage(Type=" << WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Level1=" << message.Level1
        << "; Level2=" << message.Level2
        << "; Level3=" << message.Level3
        << ")";
}

bool ITCHHandler::ProcessMWCBStatusMessage(void* buffer, size_t size)
{
    assert((size == 12) && "Invalid size of the ITCH message type 'W'");
    if (size != 12)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    MWCBStatusMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    message.BreachedLevel = *data++;

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const MWCBStatusMessage& message)
{
    return stream << "MWCBStatusMessage(Type=" << WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; BreachedLevel=" << message.BreachedLevel
        << ")";
}

bool ITCHHandler::ProcessIPOQuotingMessage(void* buffer, size_t size)
{
    assert((size == 28) && "Invalid size of the ITCH message type 'W'");
    if (size != 28)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    IPOQuotingMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += ReadString(data, message.Stock);
    data += CppCommon::Endian::ReadBigEndian(data, message.IPOReleaseTime);
    message.IPOReleaseQualifier = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.IPOPrice);

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const IPOQuotingMessage& message)
{
    return stream << "IPOQuotingMessage(Type=" << WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << WriteString(message.Stock)
        << "; IPOReleaseTime=" << message.IPOReleaseTime
        << "; IPOReleaseQualifier=" << message.IPOReleaseQualifier
        << "; IPOPrice=" << message.IPOPrice
        << ")";
}

bool ITCHHandler::ProcessUnknownMessage(void* buffer, size_t size)
{
    assert((size > 0) && "Invalid size of the unknown ITCH message!");
    if (size == 0)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    UnknownMessage message;
    message.Type = *data;

    return HandleMessage(message);
}

std::ostream& operator<<(std::ostream& stream, const UnknownMessage& message)
{
    return stream << "UnknownMessage(Type=" << WriteChar(message.Type) << ")";
}

} // namespace ITCH
} // namespace CppTrader
