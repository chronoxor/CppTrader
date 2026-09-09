/*!
    \file itch_handler.inl
    \brief NASDAQ ITCH handler inline implementation
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/
#include <cassert>

namespace CppTrader {
namespace ITCH {

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const SystemEventMessage& message)
{
    stream << "SystemEventMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; EventCode=" << CppCommon::WriteChar(message.EventCode)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const StockDirectoryMessage& message)
{
    stream << "StockDirectoryMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; MarketCategory=" << CppCommon::WriteChar(message.MarketCategory)
        << "; FinancialStatusIndicator=" << CppCommon::WriteChar(message.FinancialStatusIndicator)
        << "; RoundLotSize=" << message.RoundLotSize
        << "; RoundLotsOnly=" << CppCommon::WriteChar(message.RoundLotsOnly)
        << "; IssueClassification=" << CppCommon::WriteChar(message.IssueClassification)
        << "; IssueSubType=" << CppCommon::WriteString(message.IssueSubType)
        << "; Authenticity=" << CppCommon::WriteChar(message.Authenticity)
        << "; ShortSaleThresholdIndicator=" << CppCommon::WriteChar(message.ShortSaleThresholdIndicator)
        << "; IPOFlag=" << CppCommon::WriteChar(message.IPOFlag)
        << "; LULDReferencePriceTier=" << CppCommon::WriteChar(message.LULDReferencePriceTier)
        << "; ETPFlag=" << CppCommon::WriteChar(message.ETPFlag)
        << "; ETPLeverageFactor=" << message.ETPLeverageFactor
        << "; InverseIndicator=" << CppCommon::WriteChar(message.InverseIndicator)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const StockTradingActionMessage& message)
{
    stream << "StockTradingActionMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; TradingState=" << CppCommon::WriteChar(message.TradingState)
        << "; Reserved=" << CppCommon::WriteChar(message.Reserved)
        << "; Reason=" << CppCommon::WriteChar(message.Reason)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const RegSHOMessage& message)
{
    stream << "RegSHOMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; RegSHOAction=" << CppCommon::WriteChar(message.RegSHOAction)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const MarketParticipantPositionMessage& message)
{
    stream << "MarketParticipantPositionMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; MPID=" << CppCommon::WriteString(message.MPID)
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; PrimaryMarketMaker=" << CppCommon::WriteChar(message.PrimaryMarketMaker)
        << "; MarketMakerMode=" << CppCommon::WriteChar(message.MarketMakerMode)
        << "; MarketParticipantState=" << CppCommon::WriteChar(message.MarketParticipantState)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const MWCBDeclineMessage& message)
{
    stream << "MWCBDeclineMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Level1=" << message.Level1
        << "; Level2=" << message.Level2
        << "; Level3=" << message.Level3
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const MWCBStatusMessage& message)
{
    stream << "MWCBStatusMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; BreachedLevel=" << message.BreachedLevel
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const IPOQuotingMessage& message)
{
    stream << "IPOQuotingMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; IPOReleaseTime=" << message.IPOReleaseTime
        << "; IPOReleaseQualifier=" << message.IPOReleaseQualifier
        << "; IPOPrice=" << message.IPOPrice
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const AddOrderMessage& message)
{
    stream << "AddOrderMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; OrderReferenceNumber=" << message.OrderReferenceNumber
        << "; BuySellIndicator=" << CppCommon::WriteChar(message.BuySellIndicator)
        << "; Shares=" << message.Shares
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; Price=" << message.Price
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const AddOrderMPIDMessage& message)
{
    stream << "AddOrderMPIDMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; OrderReferenceNumber=" << message.OrderReferenceNumber
        << "; BuySellIndicator=" << CppCommon::WriteChar(message.BuySellIndicator)
        << "; Shares=" << message.Shares
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; Price=" << message.Price
        << "; Attribution=" << CppCommon::WriteChar(message.Attribution)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const OrderExecutedMessage& message)
{
    stream << "OrderExecutedMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; OrderReferenceNumber=" << message.OrderReferenceNumber
        << "; ExecutedShares=" << message.ExecutedShares
        << "; MatchNumber=" << message.MatchNumber
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const OrderExecutedWithPriceMessage& message)
{
    stream << "OrderExecutedWithPriceMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; OrderReferenceNumber=" << message.OrderReferenceNumber
        << "; ExecutedShares=" << message.ExecutedShares
        << "; MatchNumber=" << message.MatchNumber
        << "; Printable=" << CppCommon::WriteChar(message.Printable)
        << "; ExecutionPrice=" << message.ExecutionPrice
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const OrderCancelMessage& message)
{
    stream << "OrderCancelMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; OrderReferenceNumber=" << message.OrderReferenceNumber
        << "; CanceledShares=" << message.CanceledShares
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const OrderDeleteMessage& message)
{
    stream << "OrderDeleteMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; OrderReferenceNumber=" << message.OrderReferenceNumber
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const OrderReplaceMessage& message)
{
    stream << "OrderReplaceMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; OriginalOrderReferenceNumber=" << message.OriginalOrderReferenceNumber
        << "; NewOrderReferenceNumber=" << message.NewOrderReferenceNumber
        << "; Shares=" << message.Shares
        << "; Price=" << message.Price
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const TradeMessage& message)
{
    stream << "TradeMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; OrderReferenceNumber=" << message.OrderReferenceNumber
        << "; BuySellIndicator=" << CppCommon::WriteChar(message.BuySellIndicator)
        << "; Shares=" << message.Shares
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; Price=" << message.Price
        << "; MatchNumber=" << message.MatchNumber
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const CrossTradeMessage& message)
{
    stream << "CrossTradeMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Shares=" << message.Shares
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; CrossPrice=" << message.CrossPrice
        << "; MatchNumber=" << message.MatchNumber
        << "; CrossType=" << CppCommon::WriteChar(message.CrossType)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const BrokenTradeMessage& message)
{
    stream << "BrokenTradeMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; MatchNumber=" << message.MatchNumber
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const NOIIMessage& message)
{
    stream << "NOIIMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; PairedShares=" << message.PairedShares
        << "; ImbalanceShares=" << message.ImbalanceShares
        << "; ImbalanceDirection=" << CppCommon::WriteChar(message.ImbalanceDirection)
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; FarPrice=" << message.FarPrice
        << "; NearPrice=" << message.NearPrice
        << "; CurrentReferencePrice=" << message.CurrentReferencePrice
        << "; CrossType=" << CppCommon::WriteChar(message.CrossType)
        << "; PriceVariationIndicator=" << CppCommon::WriteChar(message.PriceVariationIndicator)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const RPIIMessage& message)
{
    stream << "RPIIMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; InterestFlag=" << CppCommon::WriteChar(message.InterestFlag)
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const LULDAuctionCollarMessage& message)
{
    stream << "LULDAuctionCollarMessage(Type=" << CppCommon::WriteChar(message.Type)
        << "; StockLocate=" << message.StockLocate
        << "; TrackingNumber=" << message.TrackingNumber
        << "; Timestamp=" << message.Timestamp
        << "; Stock=" << CppCommon::WriteString(message.Stock)
        << "; AuctionCollarReferencePrice=" << message.AuctionCollarReferencePrice
        << "; UpperAuctionCollarPrice=" << message.UpperAuctionCollarPrice
        << "; LowerAuctionCollarPrice=" << message.LowerAuctionCollarPrice
        << "; AuctionCollarExtension=" << message.AuctionCollarExtension
        << ")";
    return stream;
}

template <class TOutputStream>
inline TOutputStream& operator<<(TOutputStream& stream, const UnknownMessage& message)
{
    stream << "UnknownMessage(Type=" << CppCommon::WriteChar(message.Type) << ")";
    return stream;
}

template<typename SubClass>
bool ITCHHandler<SubClass>::Process(void* buffer, size_t size)
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

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessMessage(void* buffer, size_t size)
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

template<typename SubClass>
void ITCHHandler<SubClass>::Reset()
{
    _size = 0;
    _cache.clear();
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessSystemEventMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessStockDirectoryMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessStockTradingActionMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessRegSHOMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessMarketParticipantPositionMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessMWCBDeclineMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessMWCBStatusMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessIPOQuotingMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessAddOrderMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessAddOrderMPIDMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessOrderExecutedMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessOrderExecutedWithPriceMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessOrderCancelMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessOrderDeleteMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessOrderReplaceMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessTradeMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessCrossTradeMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessBrokenTradeMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessNOIIMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessRPIIMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessLULDAuctionCollarMessage(void* buffer, size_t size)
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

    return onMessageWrapper(message);
}

template<typename SubClass>
bool ITCHHandler<SubClass>::ProcessUnknownMessage(void* buffer, size_t size)
{
    assert((size > 0) && "Invalid size of the unknown ITCH message!");
    if (size == 0)
        return false;

    uint8_t* data = (uint8_t*)buffer;

    UnknownMessage message;
    message.Type = *data;

    return onMessageWrapper(message);
}

template <typename SubClass>
template <size_t N>
inline size_t ITCHHandler<SubClass>::ReadString(const void* buffer, char (&str)[N])
{
    std::memcpy(str, buffer, N);

    return N;
}

template<typename SubClass>
inline size_t ITCHHandler<SubClass>::ReadTimestamp(const void* buffer, uint64_t& value)
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
