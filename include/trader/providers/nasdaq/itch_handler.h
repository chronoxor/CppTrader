/*!
    \file itch_handler.h
    \brief NASDAQ ITCH handler definition
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_ITCH_HANDLER_H
#define CPPTRADER_ITCH_HANDLER_H

#include "utility/endian.h"

#include <iostream>
#include <vector>

namespace CppTrader {

/*!
    \namespace ITCH
    \brief ITCH protocol definitions
*/
namespace ITCH {

//! System Event Message
struct SystemEventMessage
{
    char Type;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp;
    char EventCode;

    friend std::ostream& operator<<(std::ostream& stream, const SystemEventMessage& message);
};

//! Stock Directory
struct StockDirectoryMessage
{
    char Type;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp;
    char Stock[8];
    char MarketCategory;
    char FinancialStatusIndicator;
    uint32_t RoundLotSize;
    char RoundLotsOnly;
    char IssueClassification;
    char IssueSubType[2];
    char Authenticity;
    char ShortSaleThresholdIndicator;
    char IPOFlag;
    char LULDReferencePriceTier;
    char ETPFlag;
    uint32_t ETPLeverageFactor;
    char InverseIndicator;

    friend std::ostream& operator<<(std::ostream& stream, const StockDirectoryMessage& message);
};

//! Stock Trading Action
struct StockTradingActionMessage
{
    char Type;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp;
    char Stock[8];
    char TradingState;
    char Reserved;
    char Reason;

    friend std::ostream& operator<<(std::ostream& stream, const StockTradingActionMessage& message);
};

//! Reg SHO Short Sale Price Test Restricted Indicator
struct RegSHOMessage
{
    char Type;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp;
    char Stock[8];
    char RegSHOAction;

    friend std::ostream& operator<<(std::ostream& stream, const RegSHOMessage& message);
};

//! Market Participant Position
struct MarketParticipantPositionMessage
{
    char Type;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp;
    char MPID[4];
    char Stock[8];
    char PrimaryMarketMaker;
    char MarketMakerMode;
    char MarketParticipantState;

    friend std::ostream& operator<<(std::ostream& stream, const MarketParticipantPositionMessage& message);
};

//! MWCB Decline Level Message
struct MWCBDeclineMessage
{
    char Type;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp;
    uint64_t Level1;
    uint64_t Level2;
    uint64_t Level3;

    friend std::ostream& operator<<(std::ostream& stream, const MWCBDeclineMessage& message);
};

//! MWCB Status Message
struct MWCBStatusMessage
{
    char Type;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp;
    char BreachedLevel;

    friend std::ostream& operator<<(std::ostream& stream, const MWCBStatusMessage& message);
};

//! IPO Quoting Period Update
struct IPOQuotingMessage
{
    char Type;
    uint16_t StockLocate;
    uint16_t TrackingNumber;
    uint64_t Timestamp;
    char Stock[8];
    uint32_t IPOReleaseTime;
    char IPOReleaseQualifier;
    uint32_t IPOPrice;

    friend std::ostream& operator<<(std::ostream& stream, const IPOQuotingMessage& message);
};

//! Unknown message
struct UnknownMessage
{
    char Type;

    friend std::ostream& operator<<(std::ostream& stream, const UnknownMessage& message);
};

//! NASDAQ ITCH handler class
/*!
    NASDAQ ITCH handler is used to parse NASDAQ ITCH protocol and handle its
    messages in special handlers.

    NASDAQ ITCH protocol specification:
    http://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf

    NASDAQ ITCH protocol examples:
    ftp://emi.nasdaq.com/ITCH

    Not thread-safe.
*/
class ITCHHandler
{
public:
    ITCHHandler() { Reset(); }
    ITCHHandler(const ITCHHandler&) = delete;
    ITCHHandler(ITCHHandler&&) = default;
    virtual ~ITCHHandler() = default;

    ITCHHandler& operator=(const ITCHHandler&) = delete;
    ITCHHandler& operator=(ITCHHandler&&) = default;

    //! Process all messages from the given buffer in ITCH format and call corresponding handlers
    /*!
        \param buffer - Buffer to process
        \param size - Buffer size
        \return 'true' if the given buffer was successfully processed, 'false' if the given buffer process was failed
    */
    bool Process(void* buffer, size_t size);
    //! Process a single message from the given buffer in ITCH format and call corresponding handlers
    /*!
        \param buffer - Buffer to process
        \param size - Buffer size
        \return 'true' if the given buffer was successfully processed, 'false' if the given buffer process was failed
    */
    bool ProcessMessage(void* buffer, size_t size);

    //! Reset ITCH handler
    void Reset();

protected:
    // Message handlers
    virtual bool HandleMessage(const SystemEventMessage& message) { return true; }
    virtual bool HandleMessage(const StockDirectoryMessage& message) { return true; }
    virtual bool HandleMessage(const StockTradingActionMessage& message) { return true; }
    virtual bool HandleMessage(const RegSHOMessage& message) { return true; }
    virtual bool HandleMessage(const MarketParticipantPositionMessage& message) { return true; }
    virtual bool HandleMessage(const MWCBDeclineMessage& message) { return true; }
    virtual bool HandleMessage(const MWCBStatusMessage& message) { return true; }
    virtual bool HandleMessage(const IPOQuotingMessage& message) { return true; }
    virtual bool HandleMessage(const UnknownMessage& message) { return true; }

private:
    size_t _size;
    std::vector<uint8_t> _cache;

    bool ProcessSystemEventMessage(void* buffer, size_t size);
    bool ProcessStockDirectoryMessage(void* buffer, size_t size);
    bool ProcessStockTradingActionMessage(void* buffer, size_t size);
    bool ProcessRegSHOMessage(void* buffer, size_t size);
    bool ProcessMarketParticipantPositionMessage(void* buffer, size_t size);
    bool ProcessMWCBDeclineMessage(void* buffer, size_t size);
    bool ProcessMWCBStatusMessage(void* buffer, size_t size);
    bool ProcessIPOQuotingMessage(void* buffer, size_t size);
    bool ProcessUnknownMessage(void* buffer, size_t size);

    template <size_t N>
    size_t ReadString(const void* buffer, char (&str)[N]);
    size_t ReadTimestamp(const void* buffer, uint64_t& value);
};

/*! \example itch_handler.cpp NASDAQ ITCH handler example */

} // namespace ITCH
} // namespace CppTrader

#include "itch_handler.inl"

#endif // CPPTRADER_ITCH_HANDLER_H