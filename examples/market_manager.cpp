/*!
    \file market_manager.cpp
    \brief Market manager example
    \author Ivan Shynkarenka
    \date 04.08.2017
    \copyright MIT License
*/

#include "trader/matching/market_manager.h"
#include "trader/providers/nasdaq/itch_handler.h"

#include "system/stream.h"

#include <iostream>

using namespace CppTrader::ITCH;
using namespace CppTrader::Matching;

class MyMarketHandler : public MarketHandler
{
protected:
    void onAddSymbol(const Symbol& symbol) override
    { std::cout << "Add symbol: " << symbol << std::endl; }
    void onDeleteSymbol(const Symbol& symbol) override
    { std::cout << "Delete symbol: " << symbol << std::endl; }

    void onAddOrderBook(const OrderBook& order_book) override
    { std::cout << "Add order book: " << order_book << std::endl; }
    void onUpdateOrderBook(const OrderBook& order_book, bool top) override
    { std::cout << "Update order book: " << order_book << (top ? " - Top of the book!" : "") << std::endl; }
    void onDeleteOrderBook(const OrderBook& order_book) override
    { std::cout << "Delete order book: " << order_book << std::endl; }

    void onAddLevel(const OrderBook& order_book, const Level& level, bool top) override
    { std::cout << "Add level: " << level << (top ? " - Top of the book!" : "") << std::endl; }
    void onUpdateLevel(const OrderBook& order_book, const Level& level, bool top) override
    { std::cout << "Update level: " << level << (top ? " - Top of the book!" : "") << std::endl; }
    void onDeleteLevel(const OrderBook& order_book, const Level& level, bool top) override
    { std::cout << "Delete level: " << level << (top ? " - Top of the book!" : "") << std::endl; }

    void onAddOrder(const Order& order) override
    { std::cout << "Add order: " << order << std::endl; }
    void onUpdateOrder(const Order& order) override
    { std::cout << "Update order: " << order << std::endl; }
    void onDeleteOrder(const Order& order) override
    { std::cout << "Delete order: " << order << std::endl; }

    void onExecuteOrder(const Order& order, uint64_t price, uint64_t quantity) override
    { std::cout << "Execute order: " << order << " with price " << price << " and quantity " << quantity << std::endl; }
};

class MyITCHHandler : public ITCHHandler
{
public:
    MyITCHHandler(MarketManager& market) : _market(market) {}

protected:
    bool onMessage(const StockDirectoryMessage& message) override
    {
        Symbol symbol(message.StockLocate, message.Stock);
        _market.AddSymbol(symbol);
        _market.AddOrderBook(symbol);
        return true;
    }

    bool onMessage(const AddOrderMessage& message) override
    {
        _market.AddOrder(Order::Limit(message.OrderReferenceNumber, message.StockLocate, (message.BuySellIndicator == 'B') ? OrderSide::BUY : OrderSide::SELL, message.Price, message.Shares));
        return true;
    }

    bool onMessage(const AddOrderMPIDMessage& message) override
    {
        _market.AddOrder(Order::Limit(message.OrderReferenceNumber, message.StockLocate, (message.BuySellIndicator == 'B') ? OrderSide::BUY : OrderSide::SELL, message.Price, message.Shares));
        return true;
    }

    bool onMessage(const OrderExecutedMessage& message) override
    {
        _market.ExecuteOrder(message.OrderReferenceNumber, message.ExecutedShares);
        return true;
    }

    bool onMessage(const OrderExecutedWithPriceMessage& message) override
    {
        _market.ExecuteOrder(message.OrderReferenceNumber, message.ExecutionPrice, message.ExecutedShares);
        return true;
    }

    bool onMessage(const OrderCancelMessage& message) override
    {
        _market.ReduceOrder(message.OrderReferenceNumber, message.CanceledShares);
        return true;
    }

    bool onMessage(const OrderDeleteMessage& message) override
    {
        _market.DeleteOrder(message.OrderReferenceNumber);
        return true;
    }

    bool onMessage(const OrderReplaceMessage& message) override
    {
        _market.ReplaceOrder(message.OriginalOrderReferenceNumber, message.NewOrderReferenceNumber, message.Price, message.Shares);
        return true;
    }

private:
    MarketManager& _market;
};

int main(int argc, char** argv)
{
    MyMarketHandler market_handler;
    MarketManager market(market_handler);
    MyITCHHandler itch_handler(market);

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
