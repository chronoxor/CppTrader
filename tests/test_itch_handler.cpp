//
// Created by Ivan Shynkarenka on 24.07.2017
//

#include "test.h"

#include "trader/providers/nasdaq/itch_handler.h"

#include "filesystem/file.h"

using namespace CppCommon;
using namespace CppTrader::ITCH;

namespace {

class MyITCHHandler : public ITCHHandler
{
public:
    MyITCHHandler()
        : _messages(0),
          _errors(0)
    {}

    size_t messages() const { return _messages; }
    size_t errors() const { return _errors; }

protected:
    bool onMessage(const SystemEventMessage& message) override { ++_messages; return true; }
    bool onMessage(const StockDirectoryMessage& message) override { ++_messages; return true; }
    bool onMessage(const StockTradingActionMessage& message) override { ++_messages; return true; }
    bool onMessage(const RegSHOMessage& message) override { ++_messages; return true; }
    bool onMessage(const MarketParticipantPositionMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBDeclineMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBStatusMessage& message) override { ++_messages; return true; }
    bool onMessage(const IPOQuotingMessage& message) override { ++_messages; return true; }
    bool onMessage(const AddOrderMessage& message) override { ++_messages; return true; }
    bool onMessage(const AddOrderMPIDMessage& message) override { ++_messages; return true; }
    bool onMessage(const OrderExecutedMessage& message) override { ++_messages; return true; }
    bool onMessage(const OrderExecutedWithPriceMessage& message) override { ++_messages; return true; }
    bool onMessage(const OrderCancelMessage& message) override { ++_messages; return true; }
    bool onMessage(const OrderDeleteMessage& message) override { ++_messages; return true; }
    bool onMessage(const OrderReplaceMessage& message) override { ++_messages; return true; }
    bool onMessage(const TradeMessage& message) override { ++_messages; return true; }
    bool onMessage(const CrossTradeMessage& message) override { ++_messages; return true; }
    bool onMessage(const BrokenTradeMessage& message) override { ++_messages; return true; }
    bool onMessage(const NOIIMessage& message) override { ++_messages; return true; }
    bool onMessage(const RPIIMessage& message) override { ++_messages; return true; }
    bool onMessage(const LULDAuctionCollarMessage& message) override { ++_messages; return true; }
    bool onMessage(const UnknownMessage& message) override { ++_errors; return true; }

private:
    size_t _messages;
    size_t _errors;
};

} // namespace

TEST_CASE("ITCHHandler", "[CppTrader][Providers][NASDAQ]")
{
    MyITCHHandler itch_handler;

    // Open the input file
    File input("../../tools/itch/sample.itch");
    if (!input.IsExists())
        input = File("../tools/itch/sample.itch");
    REQUIRE(input.IsExists());
    input.Open(true, false);

    // Perform input
    size_t size;
    uint8_t buffer[8192];
    while ((size = input.Read(buffer, sizeof(buffer))) > 0)
    {
        // Process the buffer
        itch_handler.Process(buffer, size);
    }

    // Check results
    REQUIRE(itch_handler.errors() == 0);
    REQUIRE(itch_handler.messages() == 1563071);
}
