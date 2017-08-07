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

class MyMarketHandler : public CppTrader::MarketHandler
{
protected:
    void onAddSymbol(const CppTrader::Symbol& symbol) override
    { std::cout << "Add symbol: " << symbol << std::endl; }
    void onDeleteSymbol(const CppTrader::Symbol& symbol) override
    { std::cout << "Delete symbol: " << symbol << std::endl; }

    void onAddOrderBook(const CppTrader::OrderBook& order_book) override
    { std::cout << "Add order book: " << order_book << std::endl; }
    void onDeleteOrderBook(const CppTrader::OrderBook& order_book) override
    { std::cout << "Delete order book: " << order_book << std::endl; }
    void onUpdateOrderBook(const CppTrader::OrderBook& order_book, const CppTrader::Level& level, bool top) override
    { std::cout << "Update order book: " << order_book << " - "<< level << (top ? " - Top of the book!" : "") << std::endl; }

    void onAddOrder(const CppTrader::Order& order) override
    { std::cout << "Add order: " << order << std::endl; }
    void onReduceOrder(const CppTrader::Order& order, uint64_t quantity) override
    { std::cout << "Reduce order: " << order << " by " << quantity << std::endl; }
    void onModifyOrder(const CppTrader::Order& order, uint64_t new_price, uint64_t new_quantity) override
    { std::cout << "Modify order: " << order << " with new price " << new_price << " and quantity " << new_quantity << std::endl; }
    void onReplaceOrder(const CppTrader::Order& order, uint64_t new_id, uint64_t new_price, uint64_t new_quantity) override
    { std::cout << "Replace order: " << order << " with new Id " << new_id << ", price " << new_price << " and quantity " << new_quantity << std::endl; }
    void onReplaceOrder(const CppTrader::Order& order, const CppTrader::Order& new_order) override
    { std::cout << "Replace order: " << order << " with new order " << new_order << std::endl; }
    void onUpdateOrder(const CppTrader::Order& order) override
    { std::cout << "Update order: " << order << std::endl; }
    void onDeleteOrder(const CppTrader::Order& order) override
    { std::cout << "Delete order: " << order << std::endl; }

    void onExecuteOrder(const CppTrader::Order& order, uint64_t price, uint64_t quantity) override
    { std::cout << "Execute order: " << order << " with price " << price << " and quantity " << quantity << std::endl; }
};

class MyITCHHandler : public CppTrader::ITCH::ITCHHandler
{
public:
    MyITCHHandler(CppTrader::MarketManager& market) : _market(market) {}

protected:
    bool onMessage(const CppTrader::ITCH::StockDirectoryMessage& message) override
    {
        CppTrader::Symbol symbol(message.StockLocate, message.Stock);
        _market.AddSymbol(symbol);
        _market.AddOrderBook(symbol);
        return true;
    }

    bool onMessage(const CppTrader::ITCH::AddOrderMessage& message) override
    {
        _market.AddOrder(CppTrader::Order(message.OrderReferenceNumber, message.StockLocate, CppTrader::OrderType::LIMIT, (message.BuySellIndicator == 'B') ? CppTrader::OrderSide::BUY : CppTrader::OrderSide::SELL, message.Price, message.Shares));
        return true;
    }

    bool onMessage(const CppTrader::ITCH::AddOrderMPIDMessage& message) override
    {
        _market.AddOrder(CppTrader::Order(message.OrderReferenceNumber, message.StockLocate, CppTrader::OrderType::LIMIT, (message.BuySellIndicator == 'B') ? CppTrader::OrderSide::BUY : CppTrader::OrderSide::SELL, message.Price, message.Shares));
        return true;
    }

    bool onMessage(const CppTrader::ITCH::OrderExecutedMessage& message) override
    {
        _market.ExecuteOrder(message.OrderReferenceNumber, message.ExecutedShares);
        return true;
    }

    bool onMessage(const CppTrader::ITCH::OrderExecutedWithPriceMessage& message) override
    {
        _market.ExecuteOrder(message.OrderReferenceNumber, message.ExecutionPrice, message.ExecutedShares);
        return true;
    }

    bool onMessage(const CppTrader::ITCH::OrderCancelMessage& message) override
    {
        _market.ReduceOrder(message.OrderReferenceNumber, message.CanceledShares);
        return true;
    }

    bool onMessage(const CppTrader::ITCH::OrderDeleteMessage& message) override
    {
        _market.DeleteOrder(message.OrderReferenceNumber);
        return true;
    }

    bool onMessage(const CppTrader::ITCH::OrderReplaceMessage& message) override
    {
        _market.ReplaceOrder(message.OriginalOrderReferenceNumber, message.NewOrderReferenceNumber, message.Price, message.Shares);
        return true;
    }

private:
    CppTrader::MarketManager& _market;
};

int main(int argc, char** argv)
{
    MyMarketHandler market_handler;
    CppTrader::MarketManager market(market_handler);
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
