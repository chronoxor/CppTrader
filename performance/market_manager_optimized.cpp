//
// Created by Ivan Shynkarenka on 11.08.2017
//

#include "trader/providers/nasdaq/itch_handler.h"

#include "benchmark/reporter_console.h"
#include "filesystem/file.h"
#include "system/stream.h"
#include "time/timestamp.h"

#include <OptionParser.h>

#include <algorithm>
#include <vector>

using namespace CppCommon;
using namespace CppTrader;
using namespace CppTrader::ITCH;

struct Symbol
{
    uint16_t Id;
    char Name[8];

    Symbol() noexcept : Id(0)
    { std::memset(Name, 0, sizeof(Name)); }
    Symbol(uint16_t id, const char name[8]) noexcept : Id(id)
    { std::memcpy(Name, name, sizeof(Name)); }
};

enum class LevelType : uint8_t
{
    BID,
    ASK
};

struct Level
{
    LevelType Type;
    uint32_t Price;
    uint32_t Volume;
    size_t Orders;
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

enum class UpdateType : uint8_t
{
    ADD,
    UPDATE,
    DELETE
};

struct LevelUpdate
{
    UpdateType Type;
    Level Update;
    bool Top;
};

enum class OrderSide : uint8_t
{
    BUY,
    SELL
};

struct Order
{
    uint64_t Id;
    uint16_t Symbol;
    OrderSide Side;
    uint32_t Price;
    uint32_t Quantity;
    size_t Level;
};

struct PriceLevel
{
    uint32_t Price;
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

    std::pair<size_t, UpdateType> FindLevel(Order* order_ptr)
    {
        if (order_ptr->Side == OrderSide::BUY)
        {
            // Try to find required price level in the bid collection
            auto it = _bids.end();
            while (it-- != _bids.begin())
            {
                auto& price_level = *it;
                if (price_level.Price == order_ptr->Price)
                    return std::make_pair(price_level.Level, UpdateType::UPDATE);
                if (price_level.Price < order_ptr->Price)
                    break;
            }

            // Create a new price level
            size_t level_index = _levels.allocate();
            Level* level_ptr = _levels.get(level_index);
            level_ptr->Type = LevelType::BID;
            level_ptr->Price = order_ptr->Price;
            level_ptr->Volume = 0;
            level_ptr->Orders = 0;

            // Insert the price level into the bid collection
            _bids.insert(++it, PriceLevel{ level_ptr->Price, level_index });

            return std::make_pair(level_index, UpdateType::ADD);
        }
        else
        {
            // Try to find required price level in the ask collection
            auto it = _asks.end();
            while (it-- != _asks.begin())
            {
                auto& price_level = *it;
                if (price_level.Price == order_ptr->Price)
                    return std::make_pair(price_level.Level, UpdateType::UPDATE);
                if (price_level.Price > order_ptr->Price)
                    break;
            }

            // Create a new price level
            size_t level_index = _levels.allocate();
            Level* level_ptr = _levels.get(level_index);
            level_ptr->Type = LevelType::ASK;
            level_ptr->Price = order_ptr->Price;
            level_ptr->Volume = 0;
            level_ptr->Orders = 0;

            // Insert the price level into the ask collection
            _asks.insert(++it, PriceLevel{ level_ptr->Price, level_index });

            return std::make_pair(level_index, UpdateType::ADD);
        }
    }

    void DeleteLevel(Order* order_ptr)
    {
        if (order_ptr->Side == OrderSide::BUY)
        {
            auto it = _bids.end();
            while (it-- != _bids.begin())
            {
                if (it->Price == order_ptr->Price)
                {
                    // Erase the price level from the bid collection
                    _bids.erase(it);
                    break;
                }
                if (it->Price < order_ptr->Price)
                    break;
            }
        }
        else
        {
            auto it = _asks.end();
            while (it-- != _asks.begin())
            {
                if (it->Price == order_ptr->Price)
                {
                    // Erase the price level from the ask collection
                    _asks.erase(it);
                    break;
                }
                if (it->Price > order_ptr->Price)
                    break;
            }
        }

        // Release the price level
        _levels.free(order_ptr->Level);
    }

    LevelUpdate AddOrder(Order* order_ptr)
    {
        // Find the price level for the order
        std::pair<size_t, UpdateType> find_result = FindLevel(order_ptr);
        Level* level_ptr = _levels.get(find_result.first);

        // Update the price level volume
        level_ptr->Volume += order_ptr->Quantity;

        // Update the price level orders count
        ++level_ptr->Orders;

        // Cache the price level in the given order
        order_ptr->Level = find_result.first;

        // Price level was changed. Return top of the book modification flag.
        return LevelUpdate{ find_result.second, *level_ptr, (level_ptr == ((order_ptr->Side == OrderSide::BUY) ? best_bid() : best_ask())) };
    }

    LevelUpdate ReduceOrder(Order* order_ptr, uint32_t quantity)
    {
        // Find the price level for the order
        Level* level_ptr = _levels.get(order_ptr->Level);

        // Update the price level volume
        level_ptr->Volume -= quantity;

        // Update the price level orders count
        if (order_ptr->Quantity == 0)
            --level_ptr->Orders;

        LevelUpdate update = { UpdateType::UPDATE, Level(*level_ptr), (level_ptr == ((order_ptr->Side == OrderSide::BUY) ? best_bid() : best_ask())) };

        // Delete the empty price level
        if (level_ptr->Volume == 0)
        {
            DeleteLevel(order_ptr);
            update.Type = UpdateType::DELETE;
        }

        return update;
    }

    LevelUpdate DeleteOrder(Order* order_ptr)
    {
        // Find the price level for the order
        Level* level_ptr = _levels.get(order_ptr->Level);

        // Update the price level volume
        level_ptr->Volume -= order_ptr->Quantity;

        // Update the price level orders count
        --level_ptr->Orders;

        LevelUpdate update = { UpdateType::UPDATE, Level(*level_ptr), (level_ptr == ((order_ptr->Side == OrderSide::BUY) ? best_bid() : best_ask())) };

        // Delete the empty price level
        if (level_ptr->Volume == 0)
        {
            DeleteLevel(order_ptr);
            update.Type = UpdateType::DELETE;
        }

        return update;
    }
};

LevelPool OrderBook::_levels(1000000);

class MarketHandler
{
    friend class MarketManagerOptimized;

public:
    MarketHandler()
        : _updates(0),
          _symbols(0),
          _max_symbols(0),
          _order_books(0),
          _max_order_books(0),
          _max_order_book_levels(0),
          _orders(0),
          _max_orders(0),
          _add_orders(0),
          _update_orders(0),
          _delete_orders(0),
          _execute_orders(0)
    {}

    size_t updates() const { return _updates; }
    size_t max_symbols() const { return _max_symbols; }
    size_t max_order_books() const { return _max_order_books; }
    size_t max_order_book_levels() const { return _max_order_book_levels; }
    size_t max_orders() const { return _max_orders; }
    size_t add_orders() const { return _add_orders; }
    size_t update_orders() const { return _update_orders; }
    size_t delete_orders() const { return _delete_orders; }
    size_t execute_orders() const { return _execute_orders; }

protected:
    void onAddSymbol(const Symbol& symbol) { ++_updates; ++_symbols; _max_symbols = std::max(_symbols, _max_symbols); }
    void onDeleteSymbol(const Symbol& symbol) { ++_updates; --_symbols; }
    void onAddOrderBook(const OrderBook& order_book) { ++_updates; ++_order_books; _max_order_books = std::max(_order_books, _max_order_books); }
    void onUpdateOrderBook(const OrderBook& order_book, bool top) { _max_order_book_levels = std::max(std::max(order_book.bids().size(), order_book.asks().size()), _max_order_book_levels); }
    void onDeleteOrderBook(const OrderBook& order_book) { ++_updates; --_order_books; }
    void onAddLevel(const OrderBook& order_book, const Level& level, bool top) { ++_updates; }
    void onUpdateLevel(const OrderBook& order_book, const Level& level, bool top) { ++_updates; }
    void onDeleteLevel(const OrderBook& order_book, const Level& level, bool top) { ++_updates; }
    void onAddOrder(const Order& order) { ++_updates; ++_orders; _max_orders = std::max(_orders, _max_orders); ++_add_orders; }
    void onUpdateOrder(const Order& order) { ++_updates; ++_update_orders; }
    void onDeleteOrder(const Order& order) { ++_updates; --_orders; ++_delete_orders; }
    void onExecuteOrder(const Order& order, int64_t price, uint64_t quantity) { ++_updates; ++_execute_orders; }

private:
    size_t _updates;
    size_t _symbols;
    size_t _max_symbols;
    size_t _order_books;
    size_t _max_order_books;
    size_t _max_order_book_levels;
    size_t _orders;
    size_t _max_orders;
    size_t _add_orders;
    size_t _update_orders;
    size_t _delete_orders;
    size_t _execute_orders;
};

class MarketManagerOptimized
{
public:
    explicit MarketManagerOptimized(MarketHandler& market_handler) : _market_handler(market_handler)
    {
        _symbols.resize(10000);
        _order_books.resize(10000);
        _orders.resize(300000000);
    }
    MarketManagerOptimized(const MarketManagerOptimized&) = delete;
    MarketManagerOptimized(MarketManagerOptimized&&) = delete;

    MarketManagerOptimized& operator=(const MarketManagerOptimized&) = delete;
    MarketManagerOptimized& operator=(MarketManagerOptimized&&) = delete;

    const Symbol* GetSymbol(uint16_t id) const noexcept { return &_symbols[id]; }
    const OrderBook* GetOrderBook(uint16_t id) const noexcept { return &_order_books[id]; }
    const Order* GetOrder(uint64_t id) const noexcept { return &_orders[id]; }

    void AddSymbol(const Symbol& symbol)
    {
        // Insert the symbol
        _symbols[symbol.Id] = symbol;

        // Call the corresponding handler
        _market_handler.onAddSymbol(_symbols[symbol.Id]);
    }

    void DeleteSymbol(uint32_t id)
    {
        // Call the corresponding handler
        _market_handler.onDeleteSymbol(_symbols[id]);
    }

    void AddOrderBook(const Symbol& symbol)
    {
        // Insert the order book
        _order_books[symbol.Id] = OrderBook();

        // Call the corresponding handler
        _market_handler.onAddOrderBook(_order_books[symbol.Id]);
    }

    void DeleteOrderBook(uint32_t id)
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrderBook(_order_books[id]);
    }

    void AddOrder(uint64_t id, uint16_t symbol, OrderSide side, uint32_t price, uint32_t quantity)
    {
        // Insert the order
        Order* order_ptr = &_orders[id];
        order_ptr->Id = id;
        order_ptr->Symbol = symbol;
        order_ptr->Side = side;
        order_ptr->Quantity = quantity;

        // Call the corresponding handler
        _market_handler.onAddOrder(*order_ptr);

        // Add the new order into the order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
    }

    void ReduceOrder(uint64_t id, uint32_t quantity)
    {
        // Get the order to reduce
        Order* order_ptr = &_orders[id];

        // Calculate the minimal possible order quantity to reduce
        quantity = std::min(quantity, order_ptr->Quantity);

        // Reduce the order quantity
        order_ptr->Quantity -= quantity;

        // Update the order or delete the empty order
        if (order_ptr->Quantity > 0)
        {
            // Call the corresponding handler
            _market_handler.onUpdateOrder(*order_ptr);

            // Reduce the order in the order book
            OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
            UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity));
        }
        else
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);

            // Delete the order in the order book
            OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
            UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));
        }
    }

    void ModifyOrder(uint64_t id, int32_t new_price, uint32_t new_quantity)
    {
        // Get the order to modify
        Order* order_ptr = &_orders[id];

        // Delete the order from the order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));

        // Modify the order
        order_ptr->Price = new_price;
        order_ptr->Quantity = new_quantity;

        // Update the order or delete the empty order
        if (order_ptr->Quantity > 0)
        {
            // Call the corresponding handler
            _market_handler.onUpdateOrder(*order_ptr);

            // Add the modified order into the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
        }
        else
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);
        }
    }

    void ReplaceOrder(uint64_t id, uint64_t new_id, int32_t new_price, uint32_t new_quantity)
    {
        // Get the order to replace
        Order* order_ptr = &_orders[id];

        // Delete the old order from the order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));

        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        if (new_quantity > 0)
        {
            // Replace the order
            Order* new_order_ptr = &_orders[new_id];
            *new_order_ptr = *order_ptr;
            new_order_ptr->Id = new_id;
            new_order_ptr->Symbol = order_ptr->Symbol;
            new_order_ptr->Side = order_ptr->Side;
            new_order_ptr->Price = new_price;
            new_order_ptr->Quantity = new_quantity;

            // Call the corresponding handler
            _market_handler.onAddOrder(*new_order_ptr);

            // Add the modified order into the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(new_order_ptr));
        }
    }

    void ReplaceOrder(uint64_t id, uint64_t new_id, uint16_t new_symbol, OrderSide new_side, uint32_t new_price, uint32_t new_quantity)
    {
        // Get the order to replace
        Order* order_ptr = &_orders[id];

        // Delete the old order from the order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));

        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        if (new_quantity > 0)
        {
            // Replace the order
            Order* new_order_ptr = &_orders[new_id];
            new_order_ptr->Id = new_id;
            new_order_ptr->Symbol = new_symbol;
            new_order_ptr->Side = new_side;
            new_order_ptr->Price = new_price;
            new_order_ptr->Quantity = new_quantity;

            // Call the corresponding handler
            _market_handler.onAddOrder(*new_order_ptr);

            // Add the modified order into the order book
            order_book_ptr = &_order_books[new_order_ptr->Symbol];
            UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(new_order_ptr));
        }
    }

    void DeleteOrder(uint64_t id)
    {
        // Get the order to delete
        Order* order_ptr = &_orders[id];

        // Delete the order from the order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));

        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);
    }

    void ExecuteOrder(uint64_t id, uint32_t quantity)
    {
        // Get the order to execute
        Order* order_ptr = &_orders[id];

        // Calculate the minimal possible order quantity to execute
        quantity = std::min(quantity, order_ptr->Quantity);

        // Call the corresponding handler
        _market_handler.onExecuteOrder(*order_ptr, order_ptr->Price, quantity);

        // Reduce the order quantity
        order_ptr->Quantity -= quantity;

        // Reduce the order in the order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity));

        // Update the order or delete the empty order
        if (order_ptr->Quantity > 0)
        {
            // Call the corresponding handler
            _market_handler.onUpdateOrder(*order_ptr);
        }
        else
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);
        }
    }

    void ExecuteOrder(uint64_t id, int32_t price, uint32_t quantity)
    {
        // Get the order to execute
        Order* order_ptr = &_orders[id];

        // Calculate the minimal possible order quantity to execute
        quantity = std::min(quantity, order_ptr->Quantity);

        // Call the corresponding handler
        _market_handler.onExecuteOrder(*order_ptr, price, quantity);

        // Reduce the order quantity
        order_ptr->Quantity -= quantity;

        // Reduce the order in the order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity));

        // Update the order or delete the empty order
        if (order_ptr->Quantity > 0)
        {
            // Call the corresponding handler
            _market_handler.onUpdateOrder(*order_ptr);
        }
        else
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);
        }
    }

private:
    MarketHandler& _market_handler;

    std::vector<Symbol> _symbols;
    std::vector<OrderBook> _order_books;
    std::vector<Order> _orders;

    void UpdateLevel(const OrderBook& order_book, const LevelUpdate& update) const
    {
        switch (update.Type)
        {
            case UpdateType::ADD:
                _market_handler.onAddLevel(order_book, update.Update, update.Top);
                break;
            case UpdateType::UPDATE:
                _market_handler.onUpdateLevel(order_book, update.Update, update.Top);
                break;
            case UpdateType::DELETE:
                _market_handler.onDeleteLevel(order_book, update.Update, update.Top);
                break;
            default:
                break;
        }
        _market_handler.onUpdateOrderBook(order_book, update.Top);
    }
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
    bool onMessage(const StockDirectoryMessage& message) override { ++_messages; Symbol symbol(message.StockLocate, message.Stock); _market.AddSymbol(symbol); _market.AddOrderBook(symbol); return true; }
    bool onMessage(const StockTradingActionMessage& message) override { ++_messages; return true; }
    bool onMessage(const RegSHOMessage& message) override { ++_messages; return true; }
    bool onMessage(const MarketParticipantPositionMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBDeclineMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBStatusMessage& message) override { ++_messages; return true; }
    bool onMessage(const IPOQuotingMessage& message) override { ++_messages; return true; }
    bool onMessage(const AddOrderMessage& message) override { ++_messages; _market.AddOrder(message.OrderReferenceNumber, message.StockLocate, (message.BuySellIndicator == 'B') ? OrderSide::BUY : OrderSide::SELL, message.Price, message.Shares); return true; }
    bool onMessage(const AddOrderMPIDMessage& message) override { ++_messages; _market.AddOrder(message.OrderReferenceNumber, message.StockLocate, (message.BuySellIndicator == 'B') ? OrderSide::BUY : OrderSide::SELL, message.Price, message.Shares); return true; }
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

    MarketHandler market_handler;
    MarketManagerOptimized market(market_handler);
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
    size_t total_updates = market_handler.updates();

    std::cout << "Processing time: " << CppBenchmark::ReporterConsole::GenerateTimePeriod(timestamp_stop - timestamp_start) << std::endl;
    std::cout << "Total ITCH messages: " << total_messages << std::endl;
    std::cout << "ITCH message latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_messages) << std::endl;
    std::cout << "ITCH message throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " msg/s" << std::endl;
    std::cout << "Total market updates: " << total_updates << std::endl;
    std::cout << "Market update latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_updates) << std::endl;
    std::cout << "Market update throughput: " << total_updates * 1000000000 / (timestamp_stop - timestamp_start) << " upd/s" << std::endl;

    std::cout << std::endl;

    std::cout << "Market statistics: " << std::endl;
    std::cout << "Max symbols: " << market_handler.max_symbols() << std::endl;
    std::cout << "Max order books: " << market_handler.max_order_books() << std::endl;
    std::cout << "Max order book levels: " << market_handler.max_order_book_levels() << std::endl;
    std::cout << "Max orders: " << market_handler.max_orders() << std::endl;

    std::cout << std::endl;

    std::cout << "Order statistics: " << std::endl;
    std::cout << "Add order operations: " << market_handler.add_orders() << std::endl;
    std::cout << "Update order operations: " << market_handler.update_orders() << std::endl;
    std::cout << "Delete order operations: " << market_handler.delete_orders() << std::endl;
    std::cout << "Execute order operations: " << market_handler.execute_orders() << std::endl;

    return 0;
}
