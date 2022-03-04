/*
    Created by Apostolis Kasselouris based on code from Ivan Shynkarenka
    Date 9.02.2022
    Matching engine optimized version made for my Diploma Thesis
*/

#include "trader/providers/nasdaq/itch_handler.h"

#include "benchmark/reporter_console.h"
#include "filesystem/file.h"
#include "system/stream.h"
#include "time/timestamp.h"
#include "containers/list.h"

#include <OptionParser.h>

#include <algorithm>
#include <vector>


using namespace CppCommon;
using namespace CppTrader;
using namespace CppTrader::ITCH;


struct Symbol
{
    uint32_t Id;
    char Name[8];

    Symbol() noexcept : Id(0)
    { std::memset(Name, 0, sizeof(Name)); }
    Symbol(uint32_t id, const char name[8]) noexcept : Id(id)
    { std::memcpy(Name, name, sizeof(Name)); }
};

enum class OrderSide : uint8_t
{
    BUY,
    SELL
};

struct OrderNode : public CppCommon::List<OrderNode>::Node
{
    uint64_t Id;
    uint32_t Symbol;
    OrderSide Side;
    uint32_t Price;
    uint32_t Quantity;
    size_t Level;

    //! Is the order with buy side?
    bool IsBuy() const noexcept { return Side == OrderSide::BUY; }
    //! Is the order with sell side?
    bool IsSell() const noexcept { return Side == OrderSide::SELL; }
};

enum class LevelType : uint8_t
{
    BID,
    ASK
};

struct Level
{
    //Price level orders
    CppCommon::List<OrderNode> OrderList;

    LevelType Type;
    uint32_t Price;
    uint32_t Volume;
    size_t Orders;

    //! Is the bid price level?
    bool IsBid() const noexcept { return Type == LevelType::BID; }
    //! Is the ask price level?
    bool IsAsk() const noexcept { return Type == LevelType::ASK; }
};

struct PriceLevel
{
    uint32_t Price;
    size_t Level;
};

template<typename T>
class MemoryPool
{
public:
    MemoryPool() = default;
    explicit MemoryPool(size_t reserve) { _allocated.reserve(reserve); }

    T& operator[](size_t index) { return _allocated[index]; }

    T* get(size_t index) { return &_allocated[index]; }

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

    void free_all()
    {
        _allocated.clear();
    }

private:
    std::vector<T> _allocated;
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


class OrderBook
{
    friend class MarketManagerOptimized;

public:
    typedef std::vector<PriceLevel> Levels;

    OrderBook() :
        _best_bid(nullptr),
        _best_ask(nullptr)
    {
        _bids.reserve(5000);
        _asks.reserve(5000);
    }
    OrderBook(const OrderBook&) = delete;
    OrderBook(OrderBook&&) noexcept = default;
    ~OrderBook()
    {
        for (const auto& bid : _bids)
            _levels.free(bid.Level);
        _bids.clear();
        for (const auto& ask : _asks)
            _levels.free(ask.Level);
        _asks.clear();
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
    Level* _best_bid;
    Level* _best_ask;    

    static MemoryPool<Level> _levels;

    std::pair<size_t, UpdateType> FindLevel(OrderNode* order_ptr)
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

            // Update the best_bid price level
            if(!_bids.empty())
                _best_bid = _levels.get(_bids.back().Level);
            else{
                _best_bid = nullptr;
            }

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

            // Update the best_ask price level
            if(!_asks.empty())
                _best_ask = _levels.get(_asks.back().Level);
            else{
                _best_ask = nullptr;
            }

            return std::make_pair(level_index, UpdateType::ADD);
        }
    }

    void DeleteLevel(OrderNode* order_ptr)
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
                    return; // No price level to delete found
            }
            // Update the best_bid price level
            if(!_bids.empty())
                _best_bid = _levels.get(_bids.back().Level);
            else{
                _best_bid = nullptr;
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
                    return; // No price level to delete found
            }

            // Update the best_ask price level
            if(!_asks.empty())
                _best_ask = _levels.get(_asks.back().Level);
            else{
                _best_ask = nullptr;
            }
        }

        // Release the price level
        _levels.free(order_ptr->Level);
    }

    LevelUpdate AddOrder(OrderNode* order_ptr)
    {
        // Find the price level for the order
        std::pair<size_t, UpdateType> find_result = FindLevel(order_ptr);
        Level* level_ptr = _levels.get(find_result.first);

        // Update the price level volume
        level_ptr->Volume += order_ptr->Quantity;

        // Link the new order to the orders list of the price level
        level_ptr->OrderList.push_back(*order_ptr);

        // Update the price level orders count
        ++level_ptr->Orders;

        // Cache the price level in the given order
        order_ptr->Level = find_result.first;

        // Price level was changed. Return top of the book modification flag.
        return LevelUpdate{ find_result.second, *level_ptr, (level_ptr == (order_ptr->IsBuy() ? _best_bid : _best_ask)) };
    }

    LevelUpdate ReduceOrder(OrderNode* order_ptr, uint32_t reduce_quantity)
    {
        // Find the price level for the order
        Level* level_ptr = _levels.get(order_ptr->Level);

        // Update the price level volume
        level_ptr->Volume -= reduce_quantity;

        // Unlink the empty order from the orders list of the price level and update the price level orders count
        if (order_ptr->Quantity == 0){
            level_ptr->OrderList.pop_current(*order_ptr);
            --level_ptr->Orders;
        }

        // Delete the empty price level
        UpdateType update = UpdateType::UPDATE;
        if (level_ptr->Volume == 0)
        {
            DeleteLevel(order_ptr);
            update = UpdateType::DELETE;
        }

        return LevelUpdate{ update, *level_ptr, (level_ptr == (order_ptr->IsBuy() ? _best_bid : _best_ask)) };
    }

    LevelUpdate DeleteOrder(OrderNode* order_ptr)
    {
        // Find the price level for the order
        Level* level_ptr = _levels.get(order_ptr->Level);

        // Update the price level volume
        level_ptr->Volume -= order_ptr->Quantity;

        // Unlink the empty order from the orders list of the price level
        level_ptr->OrderList.pop_current(*order_ptr);

        // Update the price level orders count
        --level_ptr->Orders;

        // Delete the empty price level
        UpdateType update = UpdateType::UPDATE;
        if (level_ptr->Volume == 0)
        {
            DeleteLevel(order_ptr);
            update = UpdateType::DELETE;
        }

        return LevelUpdate{ update, *level_ptr, (level_ptr == (order_ptr->IsBuy() ? _best_bid : _best_ask)) };
    }
};

MemoryPool<Level> OrderBook::_levels(2000000);

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
          _max_vector_levels(0),
          _max_level_orders(0),
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
    size_t max_vector_levels() const { return _max_vector_levels; }
    size_t max_level_orders() const { return _max_level_orders; }
    size_t max_orders() const { return _max_orders; }
    size_t add_orders() const { return _add_orders; }
    size_t update_orders() const { return _update_orders; }
    size_t delete_orders() const { return _delete_orders; }
    size_t execute_orders() const { return _execute_orders; }

protected:
    void onAddSymbol(const Symbol& symbol) { ++_updates; ++_symbols; _max_symbols = std::max(_symbols, _max_symbols); }
    void onDeleteSymbol(const Symbol& symbol) { ++_updates; --_symbols; }
    void onAddOrderBook(const OrderBook& order_book) { ++_updates; ++_order_books; _max_order_books = std::max(_order_books, _max_order_books); }
    void onUpdateOrderBook(const OrderBook& order_book, bool top) { _max_vector_levels = std::max(std::max(order_book.bids().size(), order_book.asks().size()), _max_vector_levels); }
    void onDeleteOrderBook(const OrderBook& order_book) { ++_updates; --_order_books; }
    void onAddLevel(const OrderBook& order_book, const Level& level, bool top) { ++_updates; }
    void onUpdateLevel(const OrderBook& order_book, const Level& level, bool top) { ++_updates; _max_level_orders = std::max(level.Orders, _max_level_orders); }
    void onDeleteLevel(const OrderBook& order_book, const Level& level, bool top) { ++_updates; }
    void onAddOrder(const OrderNode& order) { ++_updates; ++_orders; _max_orders = std::max(_orders, _max_orders); ++_add_orders; }
    void onUpdateOrder(const OrderNode& order) { ++_updates; ++_update_orders; }
    void onDeleteOrder(const OrderNode& order) { ++_updates; --_orders; ++_delete_orders; }
    void onExecuteOrder(const OrderNode& order, int64_t price, uint64_t quantity) { ++_updates; ++_execute_orders; }

private:
    size_t _updates;
    size_t _symbols;
    size_t _max_symbols;
    size_t _order_books;
    size_t _max_order_books;
    size_t _max_vector_levels;
    size_t _max_level_orders;
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
        _orders.resize(500000000, -1);  //size, initialized value
    }
    MarketManagerOptimized(const MarketManagerOptimized&) = delete;
    MarketManagerOptimized(MarketManagerOptimized&&) = delete;

    ~MarketManagerOptimized(){
        _orders_pool.free_all();
        _symbols.clear();
        _order_books.clear();
        _orders.clear();
    }

    MarketManagerOptimized& operator=(const MarketManagerOptimized&) = delete;
    MarketManagerOptimized& operator=(MarketManagerOptimized&&) = delete;

    const Symbol* GetSymbol(uint32_t id) const noexcept { return &_symbols[id]; }
    const OrderBook* GetOrderBook(uint32_t id) const noexcept { return &_order_books[id]; }
    const OrderNode* GetOrder(uint64_t id) const noexcept { return _orders_pool.get(_orders[id]); }

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

    void AddLimitOrder(uint64_t id, uint32_t symbol, OrderSide side, uint32_t price, uint32_t quantity)
    {
         // Validate order
        if (id <= 0 || price <= 0 || quantity <= 0)
            return;

        //Find appropriate orderbook for order
        OrderBook* order_book_ptr = &_order_books[symbol];
        if (order_book_ptr == nullptr)
            return; //orderbook not found

        // Create a new order
        size_t order_index = _orders_pool.allocate();
        OrderNode* order_ptr = _orders_pool.get(order_index);
        order_ptr->Id = id;
        order_ptr->Symbol = symbol;
        order_ptr->Side = side;
        order_ptr->Price = price;
        order_ptr->Quantity = quantity;

        // Call the corresponding handler
        _market_handler.onAddOrder(*order_ptr);

        MatchOrder(order_book_ptr, order_ptr);

        if (order_ptr->Quantity > 0)
        {
            // Add the new limit order into the _orders
            if(_orders[id] == -1)
                _orders[id] = order_index;
            else{
                // Call the corresponding handler
                _market_handler.onDeleteOrder(*order_ptr);

                //delete order
                _orders_pool.free(order_index);
                
                return; //Duplicate order
            }

            // Add the new limit order into the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
        }
        else
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);

            //delete order
            _orders_pool.free(order_index);
        }
    }

    void ReduceOrder(uint64_t id, uint32_t reduce_quantity)
    {
        // Validate parameters
        if (id <= 0 || reduce_quantity <= 0)
            return;

        //Check if order to be reduced exists
        int32_t order_index = _orders[id];
        if (order_index == -1)
            return; //order not found

        // Get the order to reduce
        OrderNode* order_ptr = _orders_pool.get(order_index);

        //Get appropriate order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        if (order_book_ptr == nullptr)
            return; //orderbook not found

        // Calculate the minimal possible order quantity to reduce
        reduce_quantity = std::min(reduce_quantity, order_ptr->Quantity);

        // Reduce the order quantity
        order_ptr->Quantity -= reduce_quantity;

        if(order_ptr->Quantity > 0){
            // Call the corresponding handler
            _market_handler.onUpdateOrder(*order_ptr);

            // Reduce the order in the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, reduce_quantity));
        }else{
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);

            // Reduce the order in the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, reduce_quantity));

            //delete order
            _orders[id] = -1;
            _orders_pool.free(order_index);
        }
    }

    void ReplaceOrder(uint64_t id, uint64_t new_id, uint32_t new_price, uint32_t new_quantity)
    {
        // Validate parameters
        if (id <= 0 || new_id <= 0 || new_quantity <= 0)
            return;

        //Check if order to be replaced exists
        int32_t order_index = _orders[id];
        if (order_index == -1)
            return; //order not found

        // Get the order to replace
        OrderNode* order_ptr = _orders_pool.get(order_index);

        //Get appropriate order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        if (order_book_ptr == nullptr)
            return; //orderbook not found

        // Delete the old order from the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));

        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        // Erase the order
        _orders[id] = -1;

        // Replace the order
        order_ptr->Id = new_id;
        order_ptr->Price = new_price;
        order_ptr->Quantity = new_quantity;

        // Call the corresponding handler
        _market_handler.onAddOrder(*order_ptr);

        MatchOrder(order_book_ptr, order_ptr);

        // Add order to its appropriate order book
        if (order_ptr->Quantity > 0)
        {
            // Add the new limit order into the _orders
            if(_orders[new_id] == -1)
                _orders[new_id] = order_index;
            else{
                // Call the corresponding handler
                _market_handler.onDeleteOrder(*order_ptr);

                //delete order
                _orders_pool.free(order_index);
                
                return; //Duplicate order
            }

            // Add the new limit order into the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
        }
        else
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);

            //delete order
            _orders_pool.free(order_index);
        }
    }

    void DeleteOrder(uint64_t id)
    {
        // Validate parameters
        if (id <= 0)
            return;

        //Check if order to be deleted exists
        int32_t order_index = _orders[id];
        if (order_index == -1)
            return; //order not found

        // Get the order to delete
        OrderNode* order_ptr = _orders_pool.get(order_index);

        //Get appropriate order book
        OrderBook* order_book_ptr = &_order_books[order_ptr->Symbol];
        if (order_book_ptr == nullptr)
            return; //orderbook not found

        // Delete the order from the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));

        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        //delete order
        _orders[id] = -1;
        _orders_pool.free(order_index);
    }

    void MatchOrder(OrderBook* order_book_ptr, OrderNode* order_ptr)
    {
        // Start the matching from the top of the book
        Level* level_ptr;

        while ((level_ptr = order_ptr->IsBuy() ? order_book_ptr->_best_ask : order_book_ptr->_best_bid) != nullptr)
        {
            // Check the arbitrage bid/ask prices
            bool arbitrage = order_ptr->IsBuy() ? (order_ptr->Price >= level_ptr->Price) : (order_ptr->Price <= level_ptr->Price);
            if (!arbitrage)
                return;

            // Find the first order to execute
            OrderNode* executing_order_ptr = level_ptr->OrderList.front();

            // Execute crossed orders
            while (executing_order_ptr != nullptr)
            {
                // Find the next order to execute
                OrderNode* next_executing_order_ptr = executing_order_ptr->next;

                // Get the execution quantity and price
                uint64_t quantity = std::min(executing_order_ptr->Quantity, order_ptr->Quantity);
                uint64_t price = executing_order_ptr->Price;

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

                // Reduce the executing order in the order book
                ReduceOrder(executing_order_ptr->Id, quantity);

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*order_ptr, price, quantity);

                // Reduce the order leaves quantity
                order_ptr->Quantity -= quantity;
                if (order_ptr->Quantity == 0)
                    return;

                // Move to the next order to execute at the same price level
                executing_order_ptr = next_executing_order_ptr;
            }
        }
    }

private:
    MarketHandler& _market_handler;

    std::vector<Symbol> _symbols;
    std::vector<OrderBook> _order_books;
    std::vector<int32_t> _orders;

    static MemoryPool<OrderNode> _orders_pool;

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

MemoryPool<OrderNode> MarketManagerOptimized::_orders_pool(5000000);

class MyITCHHandler : public ITCHHandler
{
public:
    explicit MyITCHHandler(MarketManagerOptimized& market)
        : _market(market),
          _messages(0),
          _errors(0),
          _real_messages(0),
          _symbol_messages(0),
          _add_order_messages(0),
          _reduce_order_messages(0),
          _delete_order_messages(0),
          _replace_order_messages(0)
    {}

    size_t messages() const { return _messages; }
    size_t errors() const { return _errors; }
    size_t real_messages() const { return _real_messages; }
    size_t symbol_messages() const { return _symbol_messages; }
    size_t add_order_messages() const { return _add_order_messages; }
    size_t reduce_order_messages() const { return _reduce_order_messages; }
    size_t delete_order_messages() const { return _delete_order_messages; }
    size_t replace_order_messages() const { return _replace_order_messages; }

protected:
    bool onMessage(const SystemEventMessage& message) override { ++_messages; return true; }
    bool onMessage(const StockDirectoryMessage& message) override { ++_real_messages; ++_symbol_messages; ++_messages; Symbol symbol(message.StockLocate, message.Stock); _market.AddSymbol(symbol); _market.AddOrderBook(symbol); return true; }
    bool onMessage(const StockTradingActionMessage& message) override { ++_messages; return true; }
    bool onMessage(const RegSHOMessage& message) override { ++_messages; return true; }
    bool onMessage(const MarketParticipantPositionMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBDeclineMessage& message) override { ++_messages; return true; }
    bool onMessage(const MWCBStatusMessage& message) override { ++_messages; return true; }
    bool onMessage(const IPOQuotingMessage& message) override { ++_messages; return true; }
    bool onMessage(const AddOrderMessage& message) override { ++_real_messages; ++_add_order_messages; ++_messages; _market.AddLimitOrder(message.OrderReferenceNumber, message.StockLocate, (message.BuySellIndicator == 'B') ? OrderSide::BUY : OrderSide::SELL, message.Price, message.Shares); return true; }
    bool onMessage(const AddOrderMPIDMessage& message) override { ++_real_messages; ++_add_order_messages; ++_messages; _market.AddLimitOrder(message.OrderReferenceNumber, message.StockLocate, (message.BuySellIndicator == 'B') ? OrderSide::BUY : OrderSide::SELL, message.Price, message.Shares); return true; }
    bool onMessage(const OrderExecutedMessage& message) override { ++_messages; return true; }
    bool onMessage(const OrderExecutedWithPriceMessage& message) override { ++_messages; return true; }
    bool onMessage(const OrderCancelMessage& message) override { ++_real_messages; ++_reduce_order_messages; ++_messages; _market.ReduceOrder(message.OrderReferenceNumber, message.CanceledShares); return true; }
    bool onMessage(const OrderDeleteMessage& message) override { ++_real_messages; ++_delete_order_messages; ++_messages; _market.DeleteOrder(message.OrderReferenceNumber); return true; }
    bool onMessage(const OrderReplaceMessage& message) override { ++_real_messages; ++_replace_order_messages; ++_messages; _market.ReplaceOrder(message.OriginalOrderReferenceNumber, message.NewOrderReferenceNumber, message.Price, message.Shares); return true; }
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

    //load messages
    size_t _real_messages;
    size_t _symbol_messages;
    size_t _add_order_messages;
    size_t _reduce_order_messages;
    size_t _delete_order_messages;
    size_t _replace_order_messages;
};

int main(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false);

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
    uint8_t buffer[32768];  //from 8192 to 8192*4 = 32768
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
    std::cout << std::endl;
    
    std::cout << "Load messages:" << std::endl;
    std::cout << "Total ITCH messages: " << total_messages << std::endl;
    std::cout << "Total actual used ITCH messages (Real messages): " << itch_handler.real_messages() << std::endl;
    std::cout << "---------------" << std::endl;
    std::cout << "Total Symbol order messages: " << itch_handler.symbol_messages() << std::endl;
    std::cout << "Total Add order messages: " << itch_handler.add_order_messages() << std::endl;
    std::cout << "Total Reduce order messages: " << itch_handler.reduce_order_messages() << std::endl;
    std::cout << "Total Delete order messages: " << itch_handler.delete_order_messages() << std::endl;
    std::cout << "Total Replace order messages: " << itch_handler.replace_order_messages() << std::endl;
    std::cout << std::endl;

    std::cout << "Performance Statistics:" << std::endl;
    std::cout << "ITCH message latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_messages) << std::endl;
    std::cout << "ITCH message throughput: " << total_messages * 1000000000 / (timestamp_stop - timestamp_start) << " msg/s" << std::endl;
    std::cout << "Market update latency: " << CppBenchmark::ReporterConsole::GenerateTimePeriod((timestamp_stop - timestamp_start) / total_updates) << std::endl;
    std::cout << "Market update throughput: " << total_updates * 1000000000 / (timestamp_stop - timestamp_start) << " upd/s" << std::endl;
    std::cout << std::endl;

    std::cout << "Market statistics: " << std::endl;
    std::cout << "Max symbols: " << market_handler.max_symbols() << std::endl;
    std::cout << "Max order books: " << market_handler.max_order_books() << std::endl;
    std::cout << "Max vector levels: " << market_handler.max_vector_levels() << std::endl;
    std::cout << "Max level orders: " << market_handler.max_level_orders() << std::endl;
    std::cout << "Max orders: " << market_handler.max_orders() << std::endl;
    std::cout << std::endl;

    std::cout << "Order statistics: " << std::endl;
    std::cout << "Add order operations: " << market_handler.add_orders() << std::endl;
    std::cout << "Update order operations: " << market_handler.update_orders() << std::endl;
    std::cout << "Delete order operations: " << market_handler.delete_orders() << std::endl;
    std::cout << "Execute order operations: " << market_handler.execute_orders() << std::endl;
    std::cout << std::endl;

    std::cout << "Output statistics: " << std::endl;
    std::cout << "Total market updates: " << total_updates << std::endl;

    return 0;
}
