/*!
    \file itch_handler.cpp
    \brief NASDAQ ITCH handler example
    \author Ivan Shynkarenka
    \date 23.07.2017
    \copyright MIT License
*/

#include "trader/providers/nasdaq/itch_handler.h"

#include "system/stream.h"

#include <iostream>

using namespace CppTrader::ITCH;

class MyITCHHandler : public ITCHHandler<MyITCHHandler>
{
public:
    bool onMessage(const SystemEventMessage& message) { return OutputMessage(message); }
    bool onMessage(const StockDirectoryMessage& message) { return OutputMessage(message); }
    bool onMessage(const StockTradingActionMessage& message) { return OutputMessage(message); }
    bool onMessage(const RegSHOMessage& message) { return OutputMessage(message); }
    bool onMessage(const MarketParticipantPositionMessage& message) { return OutputMessage(message); }
    bool onMessage(const MWCBDeclineMessage& message) { return OutputMessage(message); }
    bool onMessage(const MWCBStatusMessage& message) { return OutputMessage(message); }
    bool onMessage(const IPOQuotingMessage& message) { return OutputMessage(message); }
    bool onMessage(const AddOrderMessage& message) { return OutputMessage(message); }
    bool onMessage(const AddOrderMPIDMessage& message) { return OutputMessage(message); }
    bool onMessage(const OrderExecutedMessage& message) { return OutputMessage(message); }
    bool onMessage(const OrderExecutedWithPriceMessage& message) { return OutputMessage(message); }
    bool onMessage(const OrderCancelMessage& message) { return OutputMessage(message); }
    bool onMessage(const OrderDeleteMessage& message) { return OutputMessage(message); }
    bool onMessage(const OrderReplaceMessage& message) { return OutputMessage(message); }
    bool onMessage(const TradeMessage& message) { return OutputMessage(message); }
    bool onMessage(const CrossTradeMessage& message) { return OutputMessage(message); }
    bool onMessage(const BrokenTradeMessage& message) { return OutputMessage(message); }
    bool onMessage(const NOIIMessage& message) { return OutputMessage(message); }
    bool onMessage(const RPIIMessage& message) { return OutputMessage(message); }
    bool onMessage(const LULDAuctionCollarMessage& message) { return OutputMessage(message); }
    bool onMessage(const UnknownMessage& message) { return OutputMessage(message); }

private:
    template <class TMessage>
    static bool OutputMessage(const TMessage& message)
    {
        std::cout << message << std::endl;
        return true;
    }
};

int main(int argc, char** argv)
{
    MyITCHHandler itch_handler;

    // Perform input
    size_t size;
    uint8_t buffer[8192];
    CppCommon::StdInput input;
    while ((size = input.Read(buffer, sizeof(buffer))) > 0)
    {
        // Process the buffer
        itch_handler.Process(buffer, size);
    }

    return 0;
}
