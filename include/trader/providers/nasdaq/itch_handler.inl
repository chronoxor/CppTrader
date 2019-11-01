/*!
    \file itch_handler.inl
    \brief NASDAQ ITCH handler inline implementation
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/

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
