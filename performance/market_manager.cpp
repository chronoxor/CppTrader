//
// Created by Ivan Shynkarenka on 05.08.2017
//

#include "trader/providers/nasdaq/itch_handler.h"
#include "trader/market_manager.h"

#include "benchmark/reporter_console.h"
#include "filesystem/file.h"
#include "system/stream.h"
#include "time/timestamp.h"

#include "../modules/cpp-optparse/OptionParser.h"

using namespace CppCommon;
using namespace CppTrader;
using namespace CppTrader::ITCH;

class MyMarketHandler : public MarketHandler
{
public:
    MyMarketHandler() : _updates(0) {}

    size_t updates() const { return _updates; }

protected:
    void onAddSymbol(const Symbol& symbol) override { ++_updates; }
    void onDeleteSymbol(const Symbol& symbol) override { ++_updates; }
    void onAddOrderBook(const OrderBook& order_book) override { ++_updates; }
    void onDeleteOrderBook(const OrderBook& order_book) override { ++_updates; }
    void onUpdateOrderBook(const OrderBook& order_book, const Level& level, bool top) override { ++_updates; }
    void onAddOrder(const Order& order) override { ++_updates; }
    void onReduceOrder(const Order& order, uint64_t quantity) override { ++_updates; }
    void onModifyOrder(const Order& order, uint64_t new_price, uint64_t new_quantity) override { ++_updates; }
    void onReplaceOrder(const Order& order, uint64_t new_id, uint64_t new_price, uint64_t new_quantity) override { ++_updates; }
    void onReplaceOrder(const Order& order, const Order& new_order) override { ++_updates; }
    void onUpdateOrder(const Order& order) override { ++_updates; }
    void onDeleteOrder(const Order& order) override { ++_updates; }
    void onExecuteOrder(const Order& order, uint64_t price, uint64_t quantity) override { ++_updates; }

private:
    size_t _updates;
};

class MyITCHHandler : public ITCHHandler
{
public:
    MyITCHHandler(MarketManager& market) : _market(market), _messages(0), _errors(0) {}

    size_t messages() const { return _messages; }
    size_t errors() const { return _errors; }

protected:
    bool onMessage(const SystemEventMessage& message) override { ++_messages; return true; }
    bool onMessage(const StockDirectoryMessage& message) override { ++_messages; Symbol symbol(message.StockLocate, message.Stock); _market.AddSymbol(symbol); _market.AddOrderBook(symbol); return true; }
    bool onMessage(const StockTradingActionMessage& message) override { ++_messages; return true; }
    bool onMessage(const RegSHOMessage& message) override { ++_messages; return true; }
    bool onMessage(const MarketParticipantPositionMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBDeclineMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBStatusMessage& message) override { ++_messages; return true; }
    bool onMessage(const IPOQuotingMessage& message) override { ++_messages; return true; }
    bool onMessage(const AddOrderMessage& message) override { ++_messages; _market.AddOrder(Order(message.OrderReferenceNumber, message.StockLocate, OrderType::LIMIT, (message.BuySellIndicator == 'B') ? OrderSide::BUY : OrderSide::SELL, message.Price, message.Shares)); return true; }
    bool onMessage(const AddOrderMPIDMessage& message) override { ++_messages; _market.AddOrder(Order(message.OrderReferenceNumber, message.StockLocate, OrderType::LIMIT, (message.BuySellIndicator == 'B') ? OrderSide::BUY : OrderSide::SELL, message.Price, message.Shares)); return true; }
    bool onMessage(const OrderExecutedMessage& message) override { ++_messages; _market.ExecuteOrder(message.OrderReferenceNumber, message.ExecutedShares); return true; }
    bool onMessage(const OrderExecutedWithPriceMessage& message) override { ++_messages; _market.ExecuteOrder(message.OrderReferenceNumber, message.ExecutionPrice, message.ExecutedShares); return true; }
    bool onMessage(const OrderCancelMessage& message) override { ++_messages; _market.ReduceOrder(message.OrderReferenceNumber, message.CanceledShares); return true; }
    bool onMessage(const OrderDeleteMessage& message) override { ++_messages; _market.DeleteOrder(message.OrderReferenceNumber); return true; }
    bool onMessage(const OrderReplaceMessage& message) override { ++_messages; _market.ReplaceOrder(message.OriginalOrderReferenceNumber, message.NewOrderReferenceNumber, message.Price, message.Shares); return true; }
    bool onMessage(const TradeMessage& message) override { ++_messages; return true; }
    bool onMessage(const CrossTradeMessage& message) override { ++_messages; return true; }
    bool onMessage(const BrokenTradeMessage& message) override { ++_messages; return true; }
    bool onMessage(const NOIIMessage& message) override { ++_messages; return true; }
    bool onMessage(const RPIIMessage& message) override { ++_messages; return true; }
    bool onMessage(const UnknownMessage& message) override { ++_errors; return true; }

private:
    MarketManager& _market;
    size_t _messages;
    size_t _errors;
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-i", "--input").help("Input file name");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    MyMarketHandler market_handler;
    MarketManager market(market_handler);
    MyITCHHandler itch_handler(market);

    // Open the input file or stdin
    std::unique_ptr<Reader> input(new StdInput());
    if (options.is_set("input"))
    {
        File* file = new File(Path(options.get("input")));
        file->Open(true, false);
        input.reset(file);
    }

    // Perform input
    size_t size;
    uint8_t buffer[8192];
    std::cout << "ITCH processing...";
    uint64_t timestamp_start = Timestamp::nano();
    while ((size = input->Read(buffer, sizeof(buffer))) > 0)
    {
        // Process the buffer
        itch_handler.Process(buffer, size);
    }
    uint64_t timestamp_stop = Timestamp::nano();
    std::cout << "Done!" << std::endl;

    std::cout << std::endl;

    size_t total_updates = market_handler.updates();
    size_t total_errors = itch_handler.errors();

    std::cout << "Processing time: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total market updates: " << total_updates << std::endl;
    std::cout << "Market updates throughput: " << total_updates * 1000000000 / (timestamp_stop - timestamp_start) << " updates per second" << std::endl;
    std::cout << "Errors: " << total_errors << std::endl;

    return 0;
}
