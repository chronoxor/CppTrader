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

class MyHandler : public CppTrader::ITCH::ITCHHandler
{
protected:
    bool onMessage(const CppTrader::ITCH::SystemEventMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::StockDirectoryMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::StockTradingActionMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::RegSHOMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::MarketParticipantPositionMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::MWCBDeclineMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::MWCBStatusMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::IPOQuotingMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::AddOrderMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::AddOrderMPIDMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::OrderExecutedMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::OrderExecutedWithPriceMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::OrderCancelMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::OrderDeleteMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::OrderReplaceMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::TradeMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::CrossTradeMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::BrokenTradeMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::NOIIMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::RPIIMessage& message) override { return OutputMessage(message); }
    bool onMessage(const CppTrader::ITCH::UnknownMessage& message) override { return OutputMessage(message); }

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
    MyHandler handler;

    // Perform input
    size_t size;
    uint8_t buffer[8192];
    CppCommon::StdInput input;
    while ((size = input.Read(buffer, sizeof(buffer))) > 0)
    {
        // Process the buffer
        handler.Process(buffer, size);
    }

    return 0;
}
