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
    /// Start of Messages. Outside of time stamp messages, the start of day message is the first message sent in
    /// any trading day.
    START_OF_MESSAGES = 'O',
    /// Start of System hours. This message indicates that NASDAQ is open and ready to start accepting orders.
    START_OF_SYSTEM_HOURS = 'S',
    /// Start of Market hours. This message is intended to indicate that Market Hours orders are available
    /// for execution.
    START_OF_MARKET_HOURS = 'Q',
    /// End of Market hours. This message is intended to indicate that Market Hours orders are no longer
    /// available for execution.
    END_OF_MARKET_HOURS = 'M',
    /// End of System hours. It indicates that Nasdaq is now closed and will not accept any new orders today.
    /// It is still possible to receive Broken Trade messages and Order Delete messages after the End of Day.
    END_OF_SYSTEM_HOURS = 'E',
    /// End of Messages. This is always the last message sent in any trading day.
    END_OF_MESSAGES = 'C'
};
std::ostream& operator<<(std::ostream& stream, SystemEventCodes e);

//! The system event message type is used to signal a market or data feed handler event.
struct SystemEventMessage
{
    /// Always 0
    uint16_t StockLocate;
    /// Nasdaq internal tracking number
    uint16_t TrackingNumber;
    /// Nanoseconds since midnight
    uint32_t Timestamp;
    /// System event code
    SystemEventCodes EventCode;
};
std::ostream& operator<<(std::ostream& stream, const SystemEventMessage& message);

//! Unknown message
struct UnknownMessage
{
    /// Unknown message type
    uint8_t Type;
};
std::ostream& operator<<(std::ostream& stream, const UnknownMessage& message);

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
    virtual bool HandleMessage(const UnknownMessage& message) { return true; }

private:
    size_t _size;
    std::vector<uint8_t> _cache;

    bool ProcessSystemEventMessage(void* buffer, size_t size);
    bool ProcessUnknownMessage(uint8_t type);

    static size_t ReadTimestamp(const void* buffer, uint64_t& value);
};

/*! \example itch_handler.cpp NASDAQ ITCH handler example */

} // namespace ITCH
} // namespace CppTrader

#endif // CPPTRADER_ITCH_HANDLER_H
