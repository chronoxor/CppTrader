/*!
    \file itch_handler.h
    \brief NASDAQ ITCH handler definition
    \author Ivan Shynkarenka
    \date 21.07.2017
    \copyright MIT License
*/

#ifndef CPPTRADER_ITCH_HANDLER_H
#define CPPTRADER_ITCH_HANDLER_H

#include <iostream>
#include <vector>

namespace CppTrader {

/*!
    \namespace ITCH
    \brief ITCH protocol definitions
*/
namespace ITCH {

//! System event codes
enum class SystemEventCodes
{
    /// Start of Messages. Outside of time stamp messages, the start of day
    /// message is the first message sent in any trading day.
    START_OF_MESSAGES = 'O',
    /// Start of System hours. This message indicates that NASDAQ is open and
    /// ready to start accepting orders.
    START_OF_SYSTEM_HOURS = 'S',
    /// Start of Market hours. This message is intended to indicate that Market
    /// Hours orders are available for execution.
    START_OF_MARKET_HOURS = 'Q',
    /// End of System hours. It indicates that NASDAQ is now closed and will
    /// not accept any new orders today. It is still possible to receive Broken
    /// Trade messages and Order Delete messages after the End of Day.
    END_OF_SYSTEM_HOURS = 'E',
    /// End of Messages. This is always the last message sent in any trading day.
    END_OF_MESSAGES = 'C',

    /// Emergency Market Condition - Halt: This message is sent to inform
    /// NASDAQ market participants that the EMC is in effect. No trading is
    /// allowed during the EMC.
    EMERGENCY_MARKET_CONDITION_HALT = 'A',
    /// Emergency Market Condition – Quote Only Period: This message is sent
    /// to inform NASDAQ market participants that the EMC quotation only
    /// period is in effect.
    EMERGENCY_MARKET_CONDITION_QUOTE_ONLY_PERIOD = 'R',
    /// Emergency Market Condition – Resumption: This message is sent to
    /// inform NASDAQ market participants that EMC is no longer in effect.
    EMERGENCY_MARKET_CONDITION_RESUMPTION = 'B'
};
std::ostream& operator<<(std::ostream& stream, SystemEventCodes e);

//! The system event message type is used to signal a market or data feed handler event.
struct SystemEventMessage
{
    /// Nanoseconds portion of the timestamp.
    int Timestamp;
    /// System event code.
    SystemEventCodes EventCode;
};

//! NASDAQ ITCH handler class
/*!
    NASDAQ ITCH handler is used to parse NASDAQ ITCH protocol and handle its
    messages in special handlers.

    NASDAQ ITCH protocol specification:
    http://nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTV-ITCH-V4_1.pdf

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
    ~ITCHHandler() = default;

    ITCHHandler& operator=(const ITCHHandler&) = delete;
    ITCHHandler& operator=(ITCHHandler&&) = default;

    //! Process the given buffer in ITCH format and call corresponding handlers
    /*!
        \param buffer - Buffer to process
        \param size - Buffer size
        \return 'true' if the given buffer was successfully processed, 'false' if the given buffer process was failed
    */
    bool Process(void* buffer, size_t size);
    //! Reset ITCH handler
    void Reset();

    // Message handlers
    virtual void HandleMessage(const SystemEventMessage& message) {}

private:
    size_t _size;
    std::vector<uint8_t> _cache;
};

/*! \example itch_handler.cpp NASDAQ ITCH handler example */

} // namespace ITCH
} // namespace CppTrader

#endif // CPPTRADER_ITCH_HANDLER_H
