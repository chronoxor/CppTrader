/*!
    \file itch_handler.cpp
    \brief NASDAQ ITCH handler implementation
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/

#include "trader/providers/nasdaq/itch_handler.h"

#include <cassert>

namespace CppTrader {
namespace ITCH {

bool ITCHHandler::Process(void* buffer, size_t size)
{
    size_t index = 0;
    uint8_t* data = (uint8_t*)buffer;

    while (index < size)
    {
        if (_size == 0)
        {
            size_t remaining = size - index;

            // Collect message size into the cache
            if (((_cache.size() == 0) && (remaining < 3)) || (_cache.size() == 1))
            {
                _cache.push_back(data[index++]);
                continue;
            }

            // Read a new message size
            uint16_t message_size;
            if (_cache.empty())
            {
                // Read the message size directly from the input buffer
                index += CppCommon::Endian::ReadBigEndian(&data[index], message_size);
            }
            else
            {
                // Read the message size from the cache
                CppCommon::Endian::ReadBigEndian(_cache.data(), message_size);

                // Clear the cache
                _cache.clear();
            }
            _size = message_size;
        }

        // Read a new message
        if (_size > 0)
        {
            size_t remaining = size - index;

            // Complete or place the message into the cache
            if (!_cache.empty())
            {
                size_t tail = _size - _cache.size();
                if (tail > remaining)
                    tail = remaining;
                _cache.insert(_cache.end(), &data[index], &data[index + tail]);
                index += tail;
                if (_cache.size() < _size)
                    continue;
            }
            else if (_size > remaining)
            {
                _cache.reserve(_size);
                _cache.insert(_cache.end(), &data[index], &data[index + remaining]);
                index += remaining;
                continue;
            }

            // Process the current message
            if (_cache.empty())
            {
                // Process the current message size directly from the input buffer
                if (!ProcessMessage(&data[index], _size))
                    return false;
                index += _size;
            }
            else
            {
                // Process the current message size directly from the cache
                if (!ProcessMessage(_cache.data(), _size))
                    return false;

                // Clear the cache
                _cache.clear();
            }

            // Process the next message
            _size = 0;
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
        case 'A':
            return ProcessAddOrderMessage(data, size);
        case 'F':
            return ProcessAddOrderMPIDMessage(data, size);
        case 'E':
            return ProcessOrderExecutedMessage(data, size);
        case 'C':
            return ProcessOrderExecutedWithPriceMessage(data, size);
        case 'X':
            return ProcessOrderCancelMessage(data, size);
        case 'D':
            return ProcessOrderDeleteMessage(data, size);
        case 'U':
            return ProcessOrderReplaceMessage(data, size);
        case 'P':
            return ProcessTradeMessage(data, size);
        case 'Q':
            return ProcessCrossTradeMessage(data, size);
        case 'B':
            return ProcessBrokenTradeMessage(data, size);
        case 'I':
            return ProcessNOIIMessage(data, size);
        case 'N':
            return ProcessRPIIMessage(data, size);
        case 'J':
            return ProcessLULDAuctionCollarMessage(data, size);
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

    return onMessage(message);
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

    return onMessage(message);
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

    return onMessage(message);
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

    return onMessage(message);
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

    return onMessage(message);
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

    return onMessage(message);
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

    return onMessage(message);
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

    return onMessage(message);
}

bool ITCHHandler::ProcessAddOrderMessage(void* buffer, size_t size)
{
    assert((size == 36) && "Invalid size of the ITCH message type 'A'");
    if (size != 36)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    AddOrderMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.OrderReferenceNumber);
    message.BuySellIndicator = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.Shares);
    data += ReadString(data, message.Stock);
    data += CppCommon::Endian::ReadBigEndian(data, message.Price);

    return onMessage(message);
}

bool ITCHHandler::ProcessAddOrderMPIDMessage(void* buffer, size_t size)
{
    assert((size == 40) && "Invalid size of the ITCH message type 'F'");
    if (size != 40)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    AddOrderMPIDMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.OrderReferenceNumber);
    message.BuySellIndicator = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.Shares);
    data += ReadString(data, message.Stock);
    data += CppCommon::Endian::ReadBigEndian(data, message.Price);
    message.Attribution = *data++;

    return onMessage(message);
}

bool ITCHHandler::ProcessOrderExecutedMessage(void* buffer, size_t size)
{
    assert((size == 31) && "Invalid size of the ITCH message type 'E'");
    if (size != 31)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    OrderExecutedMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.OrderReferenceNumber);
    data += CppCommon::Endian::ReadBigEndian(data, message.ExecutedShares);
    data += CppCommon::Endian::ReadBigEndian(data, message.MatchNumber);

    return onMessage(message);
}

bool ITCHHandler::ProcessOrderExecutedWithPriceMessage(void* buffer, size_t size)
{
    assert((size == 36) && "Invalid size of the ITCH message type 'C'");
    if (size != 36)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    OrderExecutedWithPriceMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.OrderReferenceNumber);
    data += CppCommon::Endian::ReadBigEndian(data, message.ExecutedShares);
    data += CppCommon::Endian::ReadBigEndian(data, message.MatchNumber);
    message.Printable = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.ExecutionPrice);

    return onMessage(message);
}

bool ITCHHandler::ProcessOrderCancelMessage(void* buffer, size_t size)
{
    assert((size == 23) && "Invalid size of the ITCH message type 'X'");
    if (size != 23)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    OrderCancelMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.OrderReferenceNumber);
    data += CppCommon::Endian::ReadBigEndian(data, message.CanceledShares);

    return onMessage(message);
}

bool ITCHHandler::ProcessOrderDeleteMessage(void* buffer, size_t size)
{
    assert((size == 19) && "Invalid size of the ITCH message type 'D'");
    if (size != 19)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    OrderDeleteMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.OrderReferenceNumber);

    return onMessage(message);
}

bool ITCHHandler::ProcessOrderReplaceMessage(void* buffer, size_t size)
{
    assert((size == 35) && "Invalid size of the ITCH message type 'U'");
    if (size != 35)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    OrderReplaceMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.OriginalOrderReferenceNumber);
    data += CppCommon::Endian::ReadBigEndian(data, message.NewOrderReferenceNumber);
    data += CppCommon::Endian::ReadBigEndian(data, message.Shares);
    data += CppCommon::Endian::ReadBigEndian(data, message.Price);

    return onMessage(message);
}

bool ITCHHandler::ProcessTradeMessage(void* buffer, size_t size)
{
    assert((size == 44) && "Invalid size of the ITCH message type 'P'");
    if (size != 44)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    TradeMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.OrderReferenceNumber);
    message.BuySellIndicator = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.Shares);
    data += ReadString(data, message.Stock);
    data += CppCommon::Endian::ReadBigEndian(data, message.Price);
    data += CppCommon::Endian::ReadBigEndian(data, message.MatchNumber);

    return onMessage(message);
}

bool ITCHHandler::ProcessCrossTradeMessage(void* buffer, size_t size)
{
    assert((size == 40) && "Invalid size of the ITCH message type 'Q'");
    if (size != 40)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    CrossTradeMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.Shares);
    data += ReadString(data, message.Stock);
    data += CppCommon::Endian::ReadBigEndian(data, message.CrossPrice);
    data += CppCommon::Endian::ReadBigEndian(data, message.MatchNumber);
    message.CrossType = *data++;

    return onMessage(message);
}

bool ITCHHandler::ProcessBrokenTradeMessage(void* buffer, size_t size)
{
    assert((size == 19) && "Invalid size of the ITCH message type 'B'");
    if (size != 19)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    BrokenTradeMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.MatchNumber);

    return onMessage(message);
}

bool ITCHHandler::ProcessNOIIMessage(void* buffer, size_t size)
{
    assert((size == 50) && "Invalid size of the ITCH message type 'I'");
    if (size != 50)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    NOIIMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += CppCommon::Endian::ReadBigEndian(data, message.PairedShares);
    data += CppCommon::Endian::ReadBigEndian(data, message.ImbalanceShares);
    message.ImbalanceDirection = *data++;
    data += ReadString(data, message.Stock);
    data += CppCommon::Endian::ReadBigEndian(data, message.FarPrice);
    data += CppCommon::Endian::ReadBigEndian(data, message.NearPrice);
    data += CppCommon::Endian::ReadBigEndian(data, message.CurrentReferencePrice);
    message.CrossType = *data++;
    message.PriceVariationIndicator = *data++;

    return onMessage(message);
}

bool ITCHHandler::ProcessRPIIMessage(void* buffer, size_t size)
{
    assert((size == 20) && "Invalid size of the ITCH message type 'N'");
    if (size != 20)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    RPIIMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += ReadString(data, message.Stock);
    message.InterestFlag = *data++;

    return onMessage(message);
}

bool ITCHHandler::ProcessLULDAuctionCollarMessage(void* buffer, size_t size)
{
    assert((size == 35) && "Invalid size of the ITCH message type 'J'");
    if (size != 35)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    LULDAuctionCollarMessage message;
    message.Type = *data++;
    data += CppCommon::Endian::ReadBigEndian(data, message.StockLocate);
    data += CppCommon::Endian::ReadBigEndian(data, message.TrackingNumber);
    data += ReadTimestamp(data, message.Timestamp);
    data += ReadString(data, message.Stock);
    data += CppCommon::Endian::ReadBigEndian(data, message.AuctionCollarReferencePrice);
    data += CppCommon::Endian::ReadBigEndian(data, message.UpperAuctionCollarPrice);
    data += CppCommon::Endian::ReadBigEndian(data, message.LowerAuctionCollarPrice);
    data += CppCommon::Endian::ReadBigEndian(data, message.AuctionCollarExtension);

    return onMessage(message);
}

bool ITCHHandler::ProcessUnknownMessage(void* buffer, size_t size)
{
    assert((size > 0) && "Invalid size of the unknown ITCH message!");
    if (size == 0)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    UnknownMessage message;
    message.Type = *data;

    return onMessage(message);
}

} // namespace ITCH
} // namespace CppTrader
