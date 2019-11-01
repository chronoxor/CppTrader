//
// Created by Ivan Shynkarenka on 12.08.2017
//

#include "trader/providers/nasdaq/itch_handler.h"

#include "benchmark/reporter_console.h"
#include "filesystem/file.h"
#include "system/stream.h"
#include "time/timestamp.h"

#include <OptionParser.h>

#include <vector>

using namespace CppCommon;
using namespace CppTrader;
using namespace CppTrader::ITCH;

struct Level
{
    int32_t Price;
    uint32_t Volume;
};

class LevelPool
{
public:
    LevelPool() = default;
    explicit LevelPool(size_t reserve) { _allocated.reserve(reserve); }

    Level& operator[](size_t index) { return _allocated[index]; }

    Level* get(size_t index) { return &_allocated[index]; }

    size_t allocate()
    {
        if (_free.empty())
        {
            size_t index = _allocated.size();
            _allocated.emplace_back();
            return index;
        }
        else
        {
            size_t index = _free.back();
            _free.pop_back();
            return index;
        }
    }

    void free(size_t index)
    {
        _free.push_back(index);
    }

private:
    std::vector<Level> _allocated;
    std::vector<size_t> _free;
};

struct Order
{
    uint16_t Symbol;
    uint32_t Quantity;
    size_t Level;
};

struct PriceLevel
{
    int32_t Price;
    size_t Level;
};

class OrderBook
{
    friend class MarketManagerOptimized;

public:
    typedef std::vector<PriceLevel> Levels;

    OrderBook() = default;
    OrderBook(const OrderBook&) = delete;
    OrderBook(OrderBook&&) noexcept = default;
    ~OrderBook()
    {
        for (const auto& bid : _bids)
            _levels.free(bid.Level);
        for (const auto& ask : _asks)
            _levels.free(ask.Level);
    }

    OrderBook& operator=(const OrderBook&) = delete;
    OrderBook& operator=(OrderBook&&) noexcept = default;

    explicit operator bool() const noexcept { return !empty(); }

    bool empty() const noexcept { return _bids.empty() && _asks.empty(); }
    size_t size() const noexcept { return _bids.size() + _asks.size(); }
    const Levels& bids() const noexcept { return _bids; }
    const Levels& asks() const noexcept { return _asks; }
    const Level* best_bid() const noexcept { return _bids.empty() ? nullptr : _levels.get(_bids.back().Level); }
    const Level* best_ask() const noexcept { return _asks.empty() ? nullptr : _levels.get(_asks.back().Level); }

private:
    Levels _bids;
    Levels _asks;

    static LevelPool _levels;

    size_t FindLevel(int32_t price)
    {
        // Choose the price level collection
        Levels* levels = (price > 0) ? &_bids : &_asks;

        // Try to find required price level in the price level collection
        auto it = levels->end();
        while (it-- != levels->begin())
        {
            auto& price_level = *it;
            if (price_level.Price == price)
                return price_level.Level;
            if (price_level.Price < price)
                break;
        }

        // Create a new price level
        size_t level_index = _levels.allocate();
        Level* level_ptr = _levels.get(level_index);
        level_ptr->Price = price;
        level_ptr->Volume = 0;

        // Insert the price level into the price level collection
        levels->insert(++it, PriceLevel{ level_ptr->Price, level_index });

        return level_index;
    }

    void DeleteLevel(Order* order_ptr)
    {
        // Find the price level for the order
        Level* level_ptr = _levels.get(order_ptr->Level);

        // Choose the price level collection
        Levels* levels = (level_ptr->Price > 0) ? &_bids : &_asks;

        // Try to find required price level in the price level collection
        auto it = levels->end();
        while (it-- != levels->begin())
        {
            if (it->Price == level_ptr->Price)
            {
                // Erase the price level from the price level collection
                levels->erase(it);
                break;
            }
        }

        // Release the price level
        _levels.free(order_ptr->Level);
    }

    void AddOrder(Order* order_ptr, int32_t price)
    {
        // Find the price level for the order
        size_t level_index = FindLevel(price);
        Level* level_ptr = _levels.get(level_index);

        // Update the price level volume
        level_ptr->Volume += order_ptr->Quantity;

        // Cache the price level in the given order
        order_ptr->Level = level_index;
    }

    void ReduceOrder(Order* order_ptr, uint32_t quantity)
    {
        // Find the price level for the order
        Level* level_ptr = _levels.get(order_ptr->Level);

        // Update the price level volume
        level_ptr->Volume -= quantity;

        // Delete the empty price level
        if (level_ptr->Volume == 0)
            DeleteLevel(order_ptr);
    }

    void DeleteOrder(Order* order_ptr)
    {
        // Find the price level for the order
        Level* level_ptr = _levels.get(order_ptr->Level);

        // Update the price level volume
        level_ptr->Volume -= order_ptr->Quantity;

        // Delete the empty price level
        if (level_ptr->Volume == 0)
            DeleteLevel(order_ptr);
    }
};

LevelPool OrderBook::_levels(1000000);

class MarketManagerOptimized
{
public:
    MarketManagerOptimized()
    {
        _order_books.resize(10000);
        _orders.resize(300000000);
    }
    MarketManagerOptimized(const MarketManagerOptimized&) = delete;
    MarketManagerOptimized(MarketManagerOptimized&&) = delete;

    MarketManagerOptimized& operator=(const MarketManagerOptimized&) = delete;
    MarketManagerOptimized& operator=(MarketManagerOptimized&&) = delete;

    const OrderBook* GetOrderBook(uint16_t id) const noexcept { return &_order_books[id]; }
    const Order* GetOrder(uint64_t id) const noexcept { return &_orders[id]; }

    void AddOrderBook(uint32_t id)
    {
        // Insert the order book
        _order_books[id] = OrderBook();
    }

    void AddOrder(uint64_t id, uint16_t symbol, int32_t price, uint32_t quantity)
    {
        // Insert the order
        Order* order_ptr = &_orders[id];
        order_ptr->Symbol = symbol;
        order_ptr->Quantity = quantity;

        // Add the new order into the order book
        _order_books[order_ptr->Symbol].AddOrder(order_ptr, price);
    }

    void ReduceOrder(uint64_t id, uint32_t quantity)
    {
        // Get the order to reduce
        Order* order_ptr = &_orders[id];

        // Reduce the order quantity
        order_ptr->Quantity -= quantity;

        // Reduce the order in the order book
        _order_books[order_ptr->Symbol].ReduceOrder(order_ptr, quantity);
    }

    void ModifyOrder(uint64_t id, int32_t new_price, uint32_t new_quantity)
    {
        // Get the order to modify
        Order* order_ptr = &_orders[id];

        // Update the price for sell orders
        Level* level_ptr = OrderBook::_levels.get(order_ptr->Level);
        if (level_ptr->Price < 0)
            new_price = -new_price;

        // Delete the order from the order book
        _order_books[order_ptr->Symbol].DeleteOrder(order_ptr);

        // Modify the order
        order_ptr->Quantity = new_quantity;

        // Update the order or delete the empty order
        if (order_ptr->Quantity > 0)
        {
            // Add the modified order into the order book
            _order_books[order_ptr->Symbol].AddOrder(order_ptr, new_price);
        }
    }

    void ReplaceOrder(uint64_t id, uint64_t new_id, int32_t new_price, uint32_t new_quantity)
    {
        // Get the order to replace
        Order* order_ptr = &_orders[id];

        // Update the price for sell orders
        Level* level_ptr = OrderBook::_levels.get(order_ptr->Level);
        if (level_ptr->Price < 0)
            new_price = -new_price;

        // Delete the old order from the order book
        _order_books[order_ptr->Symbol].DeleteOrder(order_ptr);

        if (new_quantity > 0)
        {
            // Replace the order
            Order* new_order_ptr = &_orders[new_id];
            new_order_ptr->Symbol = order_ptr->Symbol;
            new_order_ptr->Quantity = new_quantity;

            // Add the modified order into the order book
            _order_books[new_order_ptr->Symbol].AddOrder(new_order_ptr, new_price);
        }
    }

    void ReplaceOrder(uint64_t id, uint64_t new_id, uint16_t new_symbol, int32_t new_price, uint32_t new_quantity)
    {
        // Get the order to replace
        Order* order_ptr = &_orders[id];

        // Update the price for sell orders
        Level* level_ptr = OrderBook::_levels.get(order_ptr->Level);
        if (level_ptr->Price < 0)
            new_price = -new_price;

        // Delete the old order from the order book
        _order_books[order_ptr->Symbol].DeleteOrder(order_ptr);

        if (new_quantity > 0)
        {
            // Replace the order
            Order* new_order_ptr = &_orders[new_id];
            new_order_ptr->Symbol = new_symbol;
            new_order_ptr->Quantity = new_quantity;

            // Add the modified order into the order book
            _order_books[new_order_ptr->Symbol].AddOrder(new_order_ptr, new_price);
        }
    }

    void DeleteOrder(uint64_t id)
    {
        // Get the order to delete
        Order* order_ptr = &_orders[id];

        // Delete the order from the order book
        _order_books[order_ptr->Symbol].DeleteOrder(order_ptr);
    }

    void ExecuteOrder(uint64_t id, uint32_t quantity)
    {
        // Get the order to execute
        Order* order_ptr = &_orders[id];

        // Reduce the order quantity
        order_ptr->Quantity -= quantity;

        // Reduce the order in the order book
        _order_books[order_ptr->Symbol].ReduceOrder(order_ptr, quantity);
    }

    void ExecuteOrder(uint64_t id, int32_t price, uint32_t quantity)
    {
        // Get the order to execute
        Order* order_ptr = &_orders[id];

        // Reduce the order quantity
        order_ptr->Quantity -= quantity;

        // Reduce the order in the order book
        _order_books[order_ptr->Symbol].ReduceOrder(order_ptr, quantity);
    }

private:
    std::vector<OrderBook> _order_books;
    std::vector<Order> _orders;
};

class MyITCHHandler : public ITCHHandler
{
public:
    explicit MyITCHHandler(MarketManagerOptimized& market)
        : _market(market),
          _messages(0),
          _errors(0)
    {}

    size_t messages() const { return _messages; }
    size_t errors() const { return _errors; }

protected:
    bool onMessage(const SystemEventMessage& message) override { ++_messages; return true; }
    bool onMessage(const StockDirectoryMessage& message) override { ++_messages; _market.AddOrderBook(message.StockLocate); return true; }
    bool onMessage(const StockTradingActionMessage& message) override { ++_messages; return true; }
    bool onMessage(const RegSHOMessage& message) override { ++_messages; return true; }
    bool onMessage(const MarketParticipantPositionMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBDeclineMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBStatusMessage& message) override { ++_messages; return true; }
    bool onMessage(const IPOQuotingMessage& message) override { ++_messages; return true; }
    bool onMessage(const AddOrderMessage& message) override { ++_messages; _market.AddOrder(message.OrderReferenceNumber, message.StockLocate, (message.BuySellIndicator == 'B') ? (int32_t)message.Price : -((int32_t)message.Price), message.Shares); return true; }
    bool onMessage(const AddOrderMPIDMessage& message) override { ++_messages; _market.AddOrder(message.OrderReferenceNumber, message.StockLocate, (message.BuySellIndicator == 'B') ? (int32_t)message.Price : -((int32_t)message.Price), message.Shares); return true; }
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
    bool onMessage(const LULDAuctionCollarMessage& message) override { ++_messages; return true; }
    bool onMessage(const UnknownMessage& message) override { ++_errors; return true; }

private:
    MarketManagerOptimized& _market;
    size_t _messages;
    size_t _errors;
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-i", "--input").dest("input").help("Input file name");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        return 0;
    }

    MarketManagerOptimized market;
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

    std::cout << "Errors: " << itch_handler.errors() << std::endl;

    std::cout << std::endl;

    size_t total_messages = itch_handler.messages();

    std::cout << "Processing time: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total ITCH messages: " << total_messages << std::endl;
    std::cout << "ITCH message latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_messages) << std::endl;
    std::cout << "ITCH message throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " msg/s" << std::endl;

    return 0;
}
