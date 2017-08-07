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
    MyMarketHandler()
        : _updates(0),
          _symbols(0),
          _max_symbols(0),
          _order_books(0),
          _max_order_books(0),
          _max_order_book_levels(0),
          _max_order_book_orders(0),
          _orders(0),
          _max_orders(0),
          _add_order(0),
          _reduce_order(0),
          _modify_order(0),
          _replace_order(0),
          _delete_order(0),
          _execute_order(0)
    {}

    size_t updates() const { return _updates; }
    size_t max_symbols() const { return _max_symbols; }
    size_t max_order_books() const { return _max_order_books; }
    size_t max_order_book_levels() const { return _max_order_book_levels; }
    size_t max_order_book_orders() const { return _max_order_book_orders; }
    size_t max_orders() const { return _max_orders; }
    size_t add_order() const { return _add_order; }
    size_t reduce_order() const { return _reduce_order; }
    size_t modify_order() const { return _modify_order; }
    size_t replace_order() const { return _replace_order; }
    size_t delete_order() const { return _delete_order; }
    size_t execute_order() const { return _execute_order; }

protected:
    void onAddSymbol(const Symbol& symbol) override { ++_updates; ++_symbols; _max_symbols = std::max(_symbols, _max_symbols); }
    void onDeleteSymbol(const Symbol& symbol) override { ++_updates; --_symbols; }
    void onAddOrderBook(const OrderBook& order_book) override { ++_updates; ++_order_books; _max_order_books = std::max(_order_books, _max_order_books); }
    void onDeleteOrderBook(const OrderBook& order_book) override { ++_updates; --_order_books; }
    void onUpdateOrderBook(const OrderBook& order_book, const Level& level, bool top) override { ++_updates; _max_order_book_levels = std::max(std::max(order_book.bids().size(), order_book.asks().size()), _max_order_book_levels); _max_order_book_orders = std::max(level.Orders.size(), _max_order_book_orders); }
    void onAddOrder(const Order& order) override { ++_updates; ++_orders; _max_orders = std::max(_orders, _max_orders); ++_add_order; }
    void onReduceOrder(const Order& order, uint64_t quantity) override { ++_updates; ++_reduce_order; }
    void onModifyOrder(const Order& order, uint64_t new_price, uint64_t new_quantity) override { ++_updates; ++_modify_order; }
    void onReplaceOrder(const Order& order, uint64_t new_id, uint64_t new_price, uint64_t new_quantity) override { ++_updates; ++_replace_order; }
    void onReplaceOrder(const Order& order, const Order& new_order) override { ++_updates; ++_replace_order; }
    void onUpdateOrder(const Order& order) override {}
    void onDeleteOrder(const Order& order) override { ++_updates; --_orders; ++_delete_order; }
    void onExecuteOrder(const Order& order, uint64_t price, uint64_t quantity) override { ++_updates; ++_execute_order; }

private:
    size_t _updates;
    size_t _symbols;
    size_t _max_symbols;
    size_t _order_books;
    size_t _max_order_books;
    size_t _max_order_book_levels;
    size_t _max_order_book_orders;
    size_t _orders;
    size_t _max_orders;
    size_t _add_order;
    size_t _reduce_order;
    size_t _modify_order;
    size_t _replace_order;
    size_t _delete_order;
    size_t _execute_order;
};

class MyITCHHandler : public ITCHHandler
{
public:
    MyITCHHandler(MarketManager& market)
        : _market(market),
          _messages(0),
          _errors(0)
    {}

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

    size_t total_errors = itch_handler.errors();
    size_t total_messages = itch_handler.messages();
    size_t total_updates = market_handler.updates();

    std::cout << "Errors: " << total_errors << std::endl;
    std::cout << "Processing time: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total ITCH messages: " << total_messages << std::endl;
    std::cout << "ITCH messages latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_messages) << std::endl;
    std::cout << "ITCH messages throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " messages per second" << std::endl;
    std::cout << "Total market updates: " << total_updates << std::endl;
    std::cout << "Market updates latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_updates) << std::endl;
    std::cout << "Market updates throughput: " << total_updates * 1000000000 / (timestamp_stop - timestamp_start) << " updates per second" << std::endl;

    std::cout << std::endl;

    std::cout << "Market statistics: " << std::endl;
    std::cout << "Max symbols: " << market_handler.max_symbols() << std::endl;
    std::cout << "Max order books: " << market_handler.max_order_books() << std::endl;
    std::cout << "Max order book levels: " << market_handler.max_order_book_levels() << std::endl;
    std::cout << "Max order book orders: " << market_handler.max_order_book_orders() << std::endl;
    std::cout << "Max orders: " << market_handler.max_orders() << std::endl;

    std::cout << std::endl;

    std::cout << "Order statistics: " << std::endl;
    std::cout << "Add order operations: " << market_handler.add_order() << std::endl;
    std::cout << "Reduce order operations: " << market_handler.reduce_order() << std::endl;
    std::cout << "Modify order operations: " << market_handler.modify_order() << std::endl;
    std::cout << "Replace order operations: " << market_handler.replace_order() << std::endl;
    std::cout << "Delete order operations: " << market_handler.delete_order() << std::endl;
    std::cout << "Execute order operations: " << market_handler.execute_order() << std::endl;

    return 0;
}
