/*!
    \file market_manager.cpp
    \brief Market manager example
    \author Ivan Shynkarenka
    \date 04.08.2017
    \copyright MIT License
*/

#include "trader/providers/nasdaq/itch_handler.h"
#include "trader/market_manager.h"

#include "system/stream.h"

#include <iostream>

class ITCHHandler : public CppTrader::ITCH::ITCHHandler
{
public:
    ITCHHandler(CppTrader::MarketManager& market) : _market(market) {}

protected:
    bool HandleMessage(const CppTrader::ITCH::StockDirectoryMessage& message) override
    {
        _market.AddSymbol(CppTrader::Symbol(message.StockLocate, message.Stock));
        return true;
    }

    bool HandleMessage(const CppTrader::ITCH::AddOrderMessage& message) override
    {
        _market.AddOrder(CppTrader::Order(message.OrderReferenceNumber, message.StockLocate, CppTrader::OrderType::LIMIT, (message.BuySellIndicator == 'B') ? CppTrader::OrderSide::BUY : CppTrader::OrderSide::SELL, message.Price, message.Shares));
        return true;
    }

    bool HandleMessage(const CppTrader::ITCH::AddOrderMPIDMessage& message) override
    {
        _market.AddOrder(CppTrader::Order(message.OrderReferenceNumber, message.StockLocate, CppTrader::OrderType::LIMIT, (message.BuySellIndicator == 'B') ? CppTrader::OrderSide::BUY : CppTrader::OrderSide::SELL, message.Price, message.Shares));
        return true;
    }

    bool HandleMessage(const CppTrader::ITCH::OrderCancelMessage& message) override
    {
        _market.ReduceOrder(message.OrderReferenceNumber, message.CanceledShares);
        return true;
    }

    bool HandleMessage(const CppTrader::ITCH::OrderDeleteMessage& message) override
    {
        _market.DeleteOrder(message.OrderReferenceNumber);
        return true;
    }

private:
    CppTrader::MarketManager& _market;
};

int main(int argc, char** argv)
{
    CppTrader::MarketManager market;
    ITCHHandler handler(market);

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
