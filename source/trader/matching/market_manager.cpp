/*!
    \file market_manager.cpp
    \brief Market manager implementation
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

#include "trader/matching/market_manager.h"

namespace CppTrader {
namespace Matching {

MarketHandler MarketManager::_default;

MarketManager::~MarketManager()
{
    // Release orders
    for (auto& order : _orders)
        _order_pool.Release(order.second);
    _orders.clear();

    // Release order books
    for (auto order_book_ptr : _order_books)
        if (order_book_ptr != nullptr)
            _order_book_pool.Release(order_book_ptr);
    _order_books.clear();

    // Release symbols
    for (auto symbol_ptr : _symbols)
        if (symbol_ptr != nullptr)
            _symbol_pool.Release(symbol_ptr);
    _symbols.clear();
}

ErrorCode MarketManager::AddSymbol(const Symbol& symbol)
{
    // Resize the symbol container
    if (_symbols.size() <= symbol.Id)
        _symbols.resize(symbol.Id + 1, nullptr);

    // Create a new symbol
    Symbol* symbol_ptr = _symbol_pool.Create(symbol);

    // Insert the symbol
    assert((_symbols[symbol.Id] == nullptr) && "Duplicate symbol detected!");
    if (_symbols[symbol.Id] != nullptr)
    {
        // Release the symbol
        _symbol_pool.Release(symbol_ptr);
        return ErrorCode::SYMBOL_DUPLICATE;
    }
    _symbols[symbol.Id] = symbol_ptr;

    // Call the corresponding handler
    _market_handler.onAddSymbol(*symbol_ptr);

    return ErrorCode::OK;
}

ErrorCode MarketManager::DeleteSymbol(uint32_t id)
{
    assert(((id < _symbols.size()) && (_symbols[id] != nullptr)) && "Symbol not found!");
    if ((_symbols.size() <= id) || (_symbols[id] == nullptr))
        return ErrorCode::SYMBOL_NOT_FOUND;

    // Get the symbol by Id
    Symbol* symbol_ptr = _symbols[id];

    // Call the corresponding handler
    _market_handler.onDeleteSymbol(*symbol_ptr);

    // Erase the symbol
    _symbols[id] = nullptr;

    // Release the symbol
    _symbol_pool.Release(symbol_ptr);

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddOrderBook(const Symbol& symbol)
{
    assert(((symbol.Id < _symbols.size()) && (_symbols[symbol.Id] != nullptr)) && "Symbol not found!");
    if ((_symbols.size() <= symbol.Id) || (_symbols[symbol.Id] == nullptr))
        return ErrorCode::SYMBOL_NOT_FOUND;

    // Get the symbol by Id
    Symbol* symbol_ptr = _symbols[symbol.Id];

    // Resize the order book container
    if (_order_books.size() <= symbol.Id)
        _order_books.resize(symbol.Id + 1, nullptr);

    // Create a new order book
    OrderBook* order_book_ptr = _order_book_pool.Create(*symbol_ptr);

    // Insert the order book
    assert((_order_books[symbol.Id] == nullptr) && "Duplicate order book detected!");
    if (_order_books[symbol.Id] != nullptr)
    {
        // Release the order book
        _order_book_pool.Release(order_book_ptr);
        return ErrorCode::ORDER_BOOK_DUPLICATE;
    }
    _order_books[symbol.Id] = order_book_ptr;

    // Call the corresponding handler
    _market_handler.onAddOrderBook(*order_book_ptr);

    return ErrorCode::OK;
}

ErrorCode MarketManager::DeleteOrderBook(uint32_t id)
{
    assert(((id < _order_books.size()) && (_order_books[id] != nullptr)) && "Order book not found!");
    if ((_order_books.size() <= id) || (_order_books[id] == nullptr))
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    // Get the order book by Id
    OrderBook* order_book_ptr = _order_books[id];

    // Call the corresponding handler
    _market_handler.onDeleteOrderBook(*order_book_ptr);

    // Erase the order book
    _order_books[id] = nullptr;

    // Release the order book
    _order_book_pool.Release(order_book_ptr);

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddOrder(const Order& order)
{
    // Validate order parameters
    ErrorCode result = order.Validate();
    if (result != ErrorCode::OK)
        return result;

    // Add the corresponding order type
    switch (order.Type)
    {
        case OrderType::MARKET:
            return AddMarketOrder(order, false);
        case OrderType::LIMIT:
            return AddLimitOrder(order, false);
        case OrderType::STOP:
            return AddStopOrder(order, false);
        case OrderType::STOP_LIMIT:
            return AddStopLimitOrder(order, false);
        default:
            return ErrorCode::ORDER_TYPE_INVALID;
    }
}

ErrorCode MarketManager::AddMarketOrder(const Order& order, bool internal)
{
    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    // Call the corresponding handler
    _market_handler.onAddOrder(new_order);

    // Automatic order matching
    if (_matching)
        MatchMarket(order_book_ptr, &new_order);

    // Call the corresponding handler
    _market_handler.onDeleteOrder(new_order);

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, internal);

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddLimitOrder(const Order& order, bool internal)
{
    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    // Call the corresponding handler
    _market_handler.onAddOrder(new_order);

    // Automatic order matching
    if (_matching)
        MatchLimit(order_book_ptr, &new_order);

    // Add a new order or delete remaining part in case of 'Immediate-Or-Cancel'/'Fill-Or-Kill' order
    if ((new_order.Quantity > 0) && !new_order.IsIOC() && !new_order.IsFOK())
    {
        // Create a new order
        OrderNode* order_ptr = _order_pool.Create(new_order);

        // Insert the order
        if (!_orders.insert(std::make_pair(order_ptr->Id, order_ptr)).second)
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);

            // Release the order
            _order_pool.Release(order_ptr);

            return ErrorCode::ORDER_DUPLICATE;
        }

        // Add the new limit order into the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
    }
    else
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(new_order);
    }

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, internal);

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddStopOrder(const Order& order, bool internal)
{
    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    // Call the corresponding handler
    _market_handler.onAddOrder(new_order);

    // Automatic order matching
    if (_matching)
    {
        // Find the price to match the stop order
        uint64_t stop_price = new_order.IsBuy() ? std::numeric_limits<uint64_t>::max() : 0;
        LevelNode* level_ptr = new_order.IsBuy() ? order_book_ptr->_best_ask : order_book_ptr->_best_bid;
        if (level_ptr != nullptr)
            stop_price = level_ptr->Price;

        // Check the arbitrage bid/ask prices
        bool arbitrage = new_order.IsBuy() ? (new_order.StopPrice <= stop_price) : (new_order.StopPrice >= stop_price);
        if (arbitrage)
        {
            // Convert the stop order into the market order
            new_order.Type = OrderType::MARKET;
            new_order.Price = 0;
            new_order.StopPrice = 0;
            new_order.TimeInForce = new_order.IsFOK() ? OrderTimeInForce::FOK : OrderTimeInForce::IOC;

            // Call the corresponding handler
            _market_handler.onUpdateOrder(new_order);

            // Match the market order
            MatchMarket(order_book_ptr, &new_order);

            // Call the corresponding handler
            _market_handler.onDeleteOrder(new_order);

            // Automatic order matching
            if (_matching)
                Match(order_book_ptr, internal);

            return ErrorCode::OK;
        }
    }

    // Add a new order
    if (new_order.Quantity > 0)
    {
        // Create a new order
        OrderNode* order_ptr = _order_pool.Create(new_order);

        // Insert the order
        if (!_orders.insert(std::make_pair(order_ptr->Id, order_ptr)).second)
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);

            // Release the order
            _order_pool.Release(order_ptr);

            return ErrorCode::ORDER_DUPLICATE;
        }

        // Add the new stop order into the order book
        order_book_ptr->AddStopOrder(order_ptr);
    }
    else
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(new_order);
    }

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, internal);

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddStopLimitOrder(const Order& order, bool internal)
{
    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    // Call the corresponding handler
    _market_handler.onAddOrder(new_order);

    // Automatic order matching
    if (_matching)
    {
        // Find the price to match the stop-limit order
        uint64_t stop_price = new_order.IsBuy() ? std::numeric_limits<uint64_t>::max() : 0;
        LevelNode* level_ptr = new_order.IsBuy() ? order_book_ptr->_best_ask : order_book_ptr->_best_bid;
        if (level_ptr != nullptr)
            stop_price = level_ptr->Price;

        // Check the arbitrage bid/ask prices
        bool arbitrage = new_order.IsBuy() ? (new_order.StopPrice <= stop_price) : (new_order.StopPrice >= stop_price);
        if (arbitrage)
        {
            // Convert the stop-limit order into the limit order
            new_order.Type = OrderType::LIMIT;
            new_order.StopPrice = 0;

            // Call the corresponding handler
            _market_handler.onUpdateOrder(new_order);

            // Match the limit order
            MatchLimit(order_book_ptr, &new_order);

            // Add a new limit order or delete remaining part in case of 'Immediate-Or-Cancel'/'Fill-Or-Kill' order
            if ((new_order.Quantity > 0) && !new_order.IsIOC() && !new_order.IsFOK())
            {
                // Create a new order
                OrderNode* order_ptr = _order_pool.Create(new_order);

                // Insert the order
                if (!_orders.insert(std::make_pair(order_ptr->Id, order_ptr)).second)
                {
                    // Call the corresponding handler
                    _market_handler.onDeleteOrder(*order_ptr);

                    // Release the order
                    _order_pool.Release(order_ptr);

                    return ErrorCode::ORDER_DUPLICATE;
                }

                // Add the new limit order into the order book
                UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
            }
            else
            {
                // Call the corresponding handler
                _market_handler.onDeleteOrder(new_order);
            }

            // Automatic order matching
            if (_matching)
                Match(order_book_ptr, internal);

            return ErrorCode::OK;
        }
    }

    // Add a new order
    if (new_order.Quantity > 0)
    {
        // Create a new order
        OrderNode* order_ptr = _order_pool.Create(new_order);

        // Insert the order
        if (!_orders.insert(std::make_pair(order_ptr->Id, order_ptr)).second)
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);

            // Release the order
            _order_pool.Release(order_ptr);

            return ErrorCode::ORDER_DUPLICATE;
        }

        // Add the new stop order into the order book
        order_book_ptr->AddStopOrder(order_ptr);
    }
    else
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(new_order);
    }

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, internal);

    return ErrorCode::OK;
}

ErrorCode MarketManager::ReduceOrder(uint64_t id, uint64_t quantity)
{
    return ReduceOrder(id, quantity, false);
}

ErrorCode MarketManager::ReduceOrder(uint64_t id, uint64_t quantity, bool internal)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        return ErrorCode::ORDER_ID_INVALID;
    assert((quantity > 0) && "Order quantity must be greater than zero!");
    if (quantity == 0)
        return ErrorCode::ORDER_QUANTITY_INVALID;

    // Get the order to reduce
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        return ErrorCode::ORDER_NOT_FOUND;
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    // Calculate the minimal possible order quantity to reduce
    quantity = std::min(quantity, order_ptr->Quantity);

    uint64_t hidden = order_ptr->HiddenQuantity();
    uint64_t visible = order_ptr->VisibleQuantity();

    // Reduce the order quantity
    order_ptr->Quantity -= quantity;

    hidden -= order_ptr->HiddenQuantity();
    visible -= order_ptr->VisibleQuantity();

    // Update the order or delete the empty order
    if (order_ptr->Quantity > 0)
    {
        // Call the corresponding handler
        _market_handler.onUpdateOrder(*order_ptr);

        // Reduce the order in the order book
        switch (order_ptr->Type)
        {
            case OrderType::LIMIT:
                UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity, hidden, visible));
                break;
            case OrderType::STOP:
            case OrderType::STOP_LIMIT:
                order_book_ptr->ReduceStopOrder(order_ptr, quantity, hidden, visible);
                break;
            case OrderType::TRAILING_STOP:
            case OrderType::TRAILING_STOP_LIMIT:
                order_book_ptr->ReduceTrailingStopOrder(order_ptr, quantity, hidden, visible);
                break;
            default:
                assert(false && "Unsupported order type!");
                break;
        }
     }
    else
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        // Reduce the order in the order book
        switch (order_ptr->Type)
        {
            case OrderType::LIMIT:
                UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity, hidden, visible));
                break;
            case OrderType::STOP:
            case OrderType::STOP_LIMIT:
                order_book_ptr->ReduceStopOrder(order_ptr, quantity, hidden, visible);
                break;
            case OrderType::TRAILING_STOP:
            case OrderType::TRAILING_STOP_LIMIT:
                order_book_ptr->ReduceTrailingStopOrder(order_ptr, quantity, hidden, visible);
                break;
            default:
                assert(false && "Unsupported order type!");
                break;
        }

        // Erase the order
        _orders.erase(order_it);

        // Relase the order
        _order_pool.Release(order_ptr);
    }

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, internal);

    return ErrorCode::OK;
}

ErrorCode MarketManager::ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity)
{
    return ModifyOrder(id, new_price, new_quantity, false);
}

ErrorCode MarketManager::ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity, bool internal)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        return ErrorCode::ORDER_ID_INVALID;
    assert((new_quantity > 0) && "Order quantity must be greater than zero!");
    if (new_quantity == 0)
        return ErrorCode::ORDER_QUANTITY_INVALID;

    // Get the order to modify
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        return ErrorCode::ORDER_NOT_FOUND;
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    // Delete the order from the order book
    switch (order_ptr->Type)
    {
        case OrderType::LIMIT:
            UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));
            break;
        case OrderType::STOP:
        case OrderType::STOP_LIMIT:
            order_book_ptr->DeleteStopOrder(order_ptr);
            break;
        case OrderType::TRAILING_STOP:
        case OrderType::TRAILING_STOP_LIMIT:
            order_book_ptr->DeleteTrailingStopOrder(order_ptr);
            break;
        default:
            assert(false && "Unsupported order type!");
            break;
    }

    // Modify the order
    order_ptr->Price = new_price;
    order_ptr->Quantity = new_quantity;

    // Update the order
    if (order_ptr->Quantity > 0)
    {
        // Call the corresponding handler
        _market_handler.onUpdateOrder(*order_ptr);

        // Automatic order matching
        if (_matching)
            MatchLimit(order_book_ptr, order_ptr);

        // Add non empty order into the order book
        if (order_ptr->Quantity > 0)
        {
            // Add the modified order into the order book
            switch (order_ptr->Type)
            {
                case OrderType::LIMIT:
                    UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
                    break;
                case OrderType::STOP:
                case OrderType::STOP_LIMIT:
                    order_book_ptr->AddStopOrder(order_ptr);
                    break;
                case OrderType::TRAILING_STOP:
                case OrderType::TRAILING_STOP_LIMIT:
                    order_book_ptr->AddTrailingStopOrder(order_ptr);
                    break;
                default:
                    assert(false && "Unsupported order type!");
                    break;
            }
        }
    }

    // Delete the empty order
    if (order_ptr->Quantity == 0)
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        // Erase the order
        _orders.erase(order_it);

        // Relase the order
        _order_pool.Release(order_ptr);
    }

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, internal);

    return ErrorCode::OK;
}

ErrorCode MarketManager::ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity)
{
    return ReplaceOrder(id, new_id, new_price, new_quantity, false);
}

ErrorCode MarketManager::ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity, bool internal)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        return ErrorCode::ORDER_ID_INVALID;
    assert((new_id > 0) && "New order Id must be greater than zero!");
    if (new_id == 0)
        return ErrorCode::ORDER_ID_INVALID;
    assert((new_quantity > 0) && "Order quantity must be greater than zero!");
    if (new_quantity == 0)
        return ErrorCode::ORDER_QUANTITY_INVALID;

    // Get the order to replace
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        return ErrorCode::ORDER_NOT_FOUND;
    OrderNode* order_ptr = (OrderNode*)order_it->second;
    assert(order_ptr->IsLimit() && "Replace order operation is valid only for limit orders!");
    if (!order_ptr->IsLimit())
        return ErrorCode::ORDER_TYPE_INVALID;

    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    // Delete the old order from the order book
    switch (order_ptr->Type)
    {
        case OrderType::LIMIT:
            UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));
            break;
        case OrderType::STOP:
        case OrderType::STOP_LIMIT:
            order_book_ptr->DeleteStopOrder(order_ptr);
            break;
        case OrderType::TRAILING_STOP:
        case OrderType::TRAILING_STOP_LIMIT:
            order_book_ptr->DeleteTrailingStopOrder(order_ptr);
            break;
        default:
            assert(false && "Unsupported order type!");
            break;
    }

    // Call the corresponding handler
    _market_handler.onDeleteOrder(*order_ptr);

    // Erase the order
    _orders.erase(order_it);

    // Replace the order
    order_ptr->Id = new_id;
    order_ptr->Price = new_price;
    order_ptr->Quantity = new_quantity;

    // Call the corresponding handler
    _market_handler.onAddOrder(*order_ptr);

    // Automatic order matching
    if (_matching)
        MatchLimit(order_book_ptr, order_ptr);

    if (order_ptr->Quantity > 0)
    {
        // Insert the order
        if (!_orders.insert(std::make_pair(order_ptr->Id, order_ptr)).second)
        {
            // Call the corresponding handler
            _market_handler.onDeleteOrder(*order_ptr);

            // Release the order
            _order_pool.Release(order_ptr);

            return ErrorCode::ORDER_DUPLICATE;
        }

        // Add the modified order into the order book
        switch (order_ptr->Type)
        {
            case OrderType::LIMIT:
                UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
                break;
            case OrderType::STOP:
            case OrderType::STOP_LIMIT:
                order_book_ptr->AddStopOrder(order_ptr);
                break;
            case OrderType::TRAILING_STOP:
            case OrderType::TRAILING_STOP_LIMIT:
                order_book_ptr->AddTrailingStopOrder(order_ptr);
                break;
            default:
                assert(false && "Unsupported order type!");
                break;
        }
    }
    else
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        // Relase the order
        _order_pool.Release(order_ptr);
    }

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, internal);

    return ErrorCode::OK;
}

ErrorCode MarketManager::ReplaceOrder(uint64_t id, const Order& new_order)
{
    // Delete the previous order by Id
    ErrorCode result = DeleteOrder(id);
    if (result != ErrorCode::OK)
        return result;

    // Add the new order
    return AddOrder(new_order);
}

ErrorCode MarketManager::DeleteOrder(uint64_t id)
{
    return DeleteOrder(id, false);
}

ErrorCode MarketManager::DeleteOrder(uint64_t id, bool internal)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        return ErrorCode::ORDER_ID_INVALID;

    // Get the order to delete
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        return ErrorCode::ORDER_NOT_FOUND;
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    // Delete the order from the order book
    switch (order_ptr->Type)
    {
        case OrderType::LIMIT:
            UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));
            break;
        case OrderType::STOP:
        case OrderType::STOP_LIMIT:
            order_book_ptr->DeleteStopOrder(order_ptr);
            break;
        case OrderType::TRAILING_STOP:
        case OrderType::TRAILING_STOP_LIMIT:
            order_book_ptr->DeleteTrailingStopOrder(order_ptr);
            break;
        default:
            assert(false && "Unsupported order type!");
            break;
    }

    // Call the corresponding handler
    _market_handler.onDeleteOrder(*order_ptr);

    // Erase the order
    _orders.erase(order_it);

    // Relase the order
    _order_pool.Release(order_ptr);

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, internal);

    return ErrorCode::OK;
}

ErrorCode MarketManager::ExecuteOrder(uint64_t id, uint64_t quantity)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        return ErrorCode::ORDER_ID_INVALID;
    assert((quantity > 0) && "Order quantity must be greater than zero!");
    if (quantity == 0)
        return ErrorCode::ORDER_QUANTITY_INVALID;

    // Get the order to execute
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        return ErrorCode::ORDER_NOT_FOUND;
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    // Calculate the minimal possible order quantity to execute
    quantity = std::min(quantity, order_ptr->Quantity);

    // Call the corresponding handler
    _market_handler.onExecuteOrder(*order_ptr, order_ptr->Price, quantity);

    uint64_t hidden = order_ptr->HiddenQuantity();
    uint64_t visible = order_ptr->VisibleQuantity();

    // Reduce the order quantity
    order_ptr->Quantity -= quantity;

    hidden -= order_ptr->HiddenQuantity();
    visible -= order_ptr->VisibleQuantity();

    // Reduce the order in the order book
    switch (order_ptr->Type)
    {
        case OrderType::LIMIT:
            UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity, hidden, visible));
            break;
        case OrderType::STOP:
        case OrderType::STOP_LIMIT:
            order_book_ptr->ReduceStopOrder(order_ptr, quantity, hidden, visible);
            break;
        case OrderType::TRAILING_STOP:
        case OrderType::TRAILING_STOP_LIMIT:
            order_book_ptr->ReduceTrailingStopOrder(order_ptr, quantity, hidden, visible);
            break;
        default:
            assert(false && "Unsupported order type!");
            break;
    }

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

        // Erase the order
        _orders.erase(order_it);

        // Relase the order
        _order_pool.Release(order_ptr);
    }

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, false);

    return ErrorCode::OK;
}

ErrorCode MarketManager::ExecuteOrder(uint64_t id, uint64_t price, uint64_t quantity)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        return ErrorCode::ORDER_ID_INVALID;
    assert((quantity > 0) && "Order quantity must be greater than zero!");
    if (quantity == 0)
        return ErrorCode::ORDER_QUANTITY_INVALID;

    // Get the order to execute
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        return ErrorCode::ORDER_NOT_FOUND;
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    // Calculate the minimal possible order quantity to execute
    quantity = std::min(quantity, order_ptr->Quantity);

    // Call the corresponding handler
    _market_handler.onExecuteOrder(*order_ptr, price, quantity);

    uint64_t hidden = order_ptr->HiddenQuantity();
    uint64_t visible = order_ptr->VisibleQuantity();

    // Reduce the order quantity
    order_ptr->Quantity -= quantity;

    hidden -= order_ptr->HiddenQuantity();
    visible -= order_ptr->VisibleQuantity();

    // Reduce the order in the order book
    switch (order_ptr->Type)
    {
        case OrderType::LIMIT:
            UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity, hidden, visible));
            break;
        case OrderType::STOP:
        case OrderType::STOP_LIMIT:
            order_book_ptr->ReduceStopOrder(order_ptr, quantity, hidden, visible);
            break;
        case OrderType::TRAILING_STOP:
        case OrderType::TRAILING_STOP_LIMIT:
            order_book_ptr->ReduceTrailingStopOrder(order_ptr, quantity, hidden, visible);
            break;
        default:
            assert(false && "Unsupported order type!");
            break;
    }

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

        // Erase the order
        _orders.erase(order_it);

        // Relase the order
        _order_pool.Release(order_ptr);
    }

    // Automatic order matching
    if (_matching)
        Match(order_book_ptr, false);

    return ErrorCode::OK;
}

void MarketManager::Match()
{
    for (auto order_book_ptr : _order_books)
        if (order_book_ptr != nullptr)
            Match(order_book_ptr, false);
}

void MarketManager::Match(OrderBook* order_book_ptr, bool internal)
{
    // Matching loop
    for (;;)
    {
        // Check the arbitrage bid/ask prices
        while ((order_book_ptr->_best_bid != nullptr) &&
               (order_book_ptr->_best_ask != nullptr) &&
               (order_book_ptr->_best_bid->Price >= order_book_ptr->_best_ask->Price))
        {
            LevelNode* bid_level_ptr = order_book_ptr->_best_bid;
            LevelNode* ask_level_ptr = order_book_ptr->_best_ask;

            uint64_t bid_level_price = bid_level_ptr->Price;
            uint64_t ask_level_price = ask_level_ptr->Price;

            // Execute crossed orders
            while (!bid_level_ptr->OrderList.empty() && !ask_level_ptr->OrderList.empty())
            {
                // Find the order to execute and the order to reduce
                OrderNode* bid_order_ptr = bid_level_ptr->OrderList.front();
                OrderNode* ask_order_ptr = ask_level_ptr->OrderList.front();

                // Special case for 'All-Or-None' orders
                if (bid_order_ptr->IsAON() || ask_order_ptr->IsAON())
                {
                    // Calculate the matching chain
                    uint64_t chain = CalculateMatchingChain(order_book_ptr, bid_level_ptr, ask_level_ptr);

                    // Matching is not avaliable
                    if (chain == 0)
                        return;

                    // Execute orders in the matching chain
                    ExecuteMatchingChain(order_book_ptr, bid_level_ptr, bid_order_ptr->Price, chain);
                    ExecuteMatchingChain(order_book_ptr, ask_level_ptr, ask_order_ptr->Price, chain);

                    break;
                }

                // Find the order to execute and the order to reduce
                OrderNode* executing_order_ptr = bid_order_ptr;
                OrderNode* reducing_order_ptr = ask_order_ptr;
                if (executing_order_ptr->Quantity > reducing_order_ptr->Quantity)
                    std::swap(executing_order_ptr, reducing_order_ptr);

                // Get the execution quantity
                uint64_t quantity = executing_order_ptr->Quantity;

                // Get the execution price
                uint64_t price = executing_order_ptr->Price;

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

                // Delete the executing order from the order book
                DeleteOrder(executing_order_ptr->Id, true);

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*reducing_order_ptr, price, quantity);

                // Reduce the remaining order in the order book
                ReduceOrder(reducing_order_ptr->Id, quantity, true);
            }

            // Activate stop orders
            if (!internal)
            {
                ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_buy_stop(), ask_level_price);
                ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_sell_stop(), bid_level_price);
            }
        }

        // Internal matching should not activate stop orders
        if (internal)
            break;

        // Activate stop orders until there is something to activate
        if (!ActivateStopOrders(order_book_ptr))
            break;
    }
}

void MarketManager::MatchMarket(OrderBook* order_book_ptr, Order* order_ptr)
{
    // Calculate acceptable marker order price with optional slippage value
    if (order_ptr->IsBuy())
    {
        // Check if there is nothing to buy
        if (order_book_ptr->best_ask() == nullptr)
            return;

        order_ptr->Price = order_book_ptr->best_ask()->Price;
        if (order_ptr->Price > (std::numeric_limits<uint64_t>::max() - order_ptr->Slippage))
            order_ptr->Price = std::numeric_limits<uint64_t>::max();
        else
            order_ptr->Price += order_ptr->Slippage;
    }
    else
    {
        // Check if there is nothing to sell
        if (order_book_ptr->best_bid() == nullptr)
            return;

        order_ptr->Price = order_book_ptr->best_bid()->Price;
        if (order_ptr->Price < (std::numeric_limits<uint64_t>::min() + order_ptr->Slippage))
            order_ptr->Price = std::numeric_limits<uint64_t>::min();
        else
            order_ptr->Price -= order_ptr->Slippage;
    }

    // Match the market order
    MatchOrder(order_book_ptr, order_ptr);
}

void MarketManager::MatchLimit(OrderBook* order_book_ptr, Order* order_ptr)
{
    // Match the limit order
    MatchOrder(order_book_ptr, order_ptr);
}

void MarketManager::MatchOrder(OrderBook* order_book_ptr, Order* order_ptr)
{
    // Start the matching from the top of the book
    LevelNode* level_ptr;
    while ((level_ptr = order_ptr->IsBuy() ? order_book_ptr->_best_ask : order_book_ptr->_best_bid) != nullptr)
    {
        // Check the arbitrage bid/ask prices
        bool arbitrage = order_ptr->IsBuy() ? (order_ptr->Price >= level_ptr->Price) : (order_ptr->Price <= level_ptr->Price);
        if (!arbitrage)
            return;

        // Special case for 'Fill-Or-Kill'/'All-Or-None' order
        if (order_ptr->IsFOK() || order_ptr->IsAON())
        {
            // Calculate the matching chain
            uint64_t chain = CalculateMatchingChain(order_book_ptr, level_ptr, order_ptr->Price, order_ptr->Quantity);

            // Matching is not avaliable
            if (chain == 0)
                return;

            // Execute orders in the matching chain
            ExecuteMatchingChain(order_book_ptr, level_ptr, order_ptr->Price, chain);

            // Call the corresponding handler
            _market_handler.onExecuteOrder(*order_ptr, order_ptr->Price, order_ptr->Quantity);

            // Reduce quantity to execute
            order_ptr->Quantity = 0;

            return;
        }

        // Execute crossed orders
        while (!level_ptr->OrderList.empty())
        {
            // Find the order to execute
            OrderNode* executing_order_ptr = level_ptr->OrderList.front();

            // Get the execution quantity
            uint64_t quantity = std::min(executing_order_ptr->Quantity, order_ptr->Quantity);

            // Special case for 'All-Or-None' orders
            if (executing_order_ptr->IsAON() && (executing_order_ptr->Quantity > order_ptr->Quantity))
                return;

            // Get the execution price
            uint64_t price = executing_order_ptr->Price;

            // Call the corresponding handler
            _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

            // Reduce the executing order in the order book
            ReduceOrder(executing_order_ptr->Id, quantity, true);

            // Call the corresponding handler
            _market_handler.onExecuteOrder(*order_ptr, price, quantity);

            // Reduce quantity to execute
            order_ptr->Quantity -= quantity;
            if (order_ptr->Quantity == 0)
                return;
        }
    }
}

bool MarketManager::ActivateStopOrders(OrderBook* order_book_ptr)
{
    bool result = false;
    bool stop = false;

    while (!stop)
    {
        stop = true;

        // Find the price to activate buy stop orders
        uint64_t ask_stop_price = std::numeric_limits<uint64_t>::max();
        LevelNode* ask_level_ptr = order_book_ptr->_best_ask;
        if (ask_level_ptr != nullptr)
            ask_stop_price = ask_level_ptr->Price;

        // Try to activate buy stop orders
        if (ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_buy_stop(), ask_stop_price))
        {
            result = true;
            stop = false;
        }

        // Find the price to activate sell stop orders
        uint64_t bid_stop_price = 0;
        LevelNode* bid_level_ptr = order_book_ptr->_best_bid;
        if (bid_level_ptr != nullptr)
            bid_stop_price = bid_level_ptr->Price;

        // Try to activate sell stop orders
        if (ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_sell_stop(), bid_stop_price))
        {
            result = true;
            stop = false;
        }
    }

    return result;
}

bool MarketManager::ActivateStopOrders(OrderBook* order_book_ptr, LevelNode* level_ptr, uint64_t stop_price)
{
    bool result = false;

    if (level_ptr != nullptr)
    {
        // Check the arbitrage bid/ask prices
        bool arbitrage = level_ptr->IsBid() ? (stop_price <= level_ptr->Price) : (stop_price >= level_ptr->Price);
        if (!arbitrage)
            return result;

        // Activate all stop orders
        while (!level_ptr->OrderList.empty())
        {
            // Find the order to activate
            OrderNode* order_ptr = level_ptr->OrderList.pop_front();

            // Activate the stop order
            switch (order_ptr->Type)
            {
                case OrderType::STOP:
                    result = ActivateStopOrder(order_book_ptr, order_ptr);
                    break;
                case OrderType::STOP_LIMIT:
                    result = ActivateStopLimitOrder(order_book_ptr, order_ptr);
                    break;
                default:
                    assert(false && "Unsupported order type!");
                    break;

            }
        }
    }

    return result;
}

bool MarketManager::ActivateStopOrder(OrderBook* order_book_ptr, OrderNode* order_ptr)
{
    // Delete the stop order from the order book
    order_book_ptr->DeleteStopOrder(order_ptr);

    // Convert the stop order into the market order
    order_ptr->Type = OrderType::MARKET;
    order_ptr->Price = 0;
    order_ptr->StopPrice = 0;
    order_ptr->TimeInForce = order_ptr->IsFOK() ? OrderTimeInForce::FOK : OrderTimeInForce::IOC;

    // Call the corresponding handler
    _market_handler.onUpdateOrder(*order_ptr);

    // Match the market order
    MatchMarket(order_book_ptr, order_ptr);

    // Call the corresponding handler
    _market_handler.onDeleteOrder(*order_ptr);

    // Erase the order
    _orders.erase(_orders.find(order_ptr->Id));

    // Relase the order
    _order_pool.Release(order_ptr);

    return true;
}

bool MarketManager::ActivateStopLimitOrder(OrderBook* order_book_ptr, OrderNode* order_ptr)
{
    // Delete the stop order from the order book
    order_book_ptr->DeleteStopOrder(order_ptr);

    // Convert the stop-limit order into the limit order
    order_ptr->Type = OrderType::LIMIT;
    order_ptr->StopPrice = 0;

    // Call the corresponding handler
    _market_handler.onUpdateOrder(*order_ptr);

    // Match the limit order
    MatchLimit(order_book_ptr, order_ptr);

    // Add a new limit order or delete remaining part in case of 'Immediate-Or-Cancel'/'Fill-Or-Kill' order
    if ((order_ptr->Quantity > 0) && !order_ptr->IsIOC() && !order_ptr->IsFOK())
    {
        // Add the new limit order into the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
    }
    else
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        // Erase the order
        _orders.erase(_orders.find(order_ptr->Id));

        // Relase the order
        _order_pool.Release(order_ptr);
    }

    return true;
}

uint64_t MarketManager::CalculateMatchingChain(OrderBook* order_book_ptr, LevelNode* level_ptr, uint64_t price, uint64_t volume)
{
    OrderNode* order_ptr = level_ptr->OrderList.front();
    uint64_t available = 0;

    // Travel through price levels
    while (level_ptr != nullptr)
    {
        // Check the arbitrage bid/ask prices
        bool arbitrage = level_ptr->IsBid() ? (price <= level_ptr->Price) : (price >= level_ptr->Price);
        if (!arbitrage)
            return 0;

        // Travel through orders at current price levels
        while (order_ptr != nullptr)
        {
            uint64_t need = volume - available;
            uint64_t quantity = order_ptr->IsAON() ? order_ptr->Quantity : std::min(order_ptr->Quantity, need);
            available += quantity;

            // Matching is possible, return the chain size
            if (volume == available)
                return available;

            // Matching is not possible
            if (volume < available)
                return 0;

            // Take the next order
            order_ptr = order_ptr->next;
        }

        // Switch to the next price level
        if (order_ptr == nullptr)
        {
            level_ptr = order_book_ptr->GetNextLevel(level_ptr);
            if (level_ptr != nullptr)
                order_ptr = level_ptr->OrderList.front();
        }
    }

    // Matching is not available
    return 0;
}

uint64_t MarketManager::CalculateMatchingChain(OrderBook* order_book_ptr, LevelNode* bid_level_ptr, LevelNode* ask_level_ptr)
{
    LevelNode* longest_level_ptr = bid_level_ptr;
    LevelNode* shortest_level_ptr = ask_level_ptr;
    OrderNode* longest_order_ptr = bid_level_ptr->OrderList.front();
    OrderNode* shortest_order_ptr = ask_level_ptr->OrderList.front();
    uint64_t required = longest_order_ptr->Quantity;
    uint64_t available = 0;

    // Find the initial longest order chain
    if (longest_order_ptr->IsAON() && shortest_order_ptr->IsAON())
    {
        // Choose the longest 'All-Or-None' order
        if (shortest_order_ptr->Quantity > longest_order_ptr->Quantity)
        {
            required = shortest_order_ptr->Quantity;
            available = 0;
            std::swap(longest_level_ptr, shortest_level_ptr);
            std::swap(longest_order_ptr, shortest_order_ptr);
        }
    }
    else if (shortest_order_ptr->IsAON())
    {
        required = shortest_order_ptr->Quantity;
        available = 0;
        std::swap(longest_level_ptr, shortest_level_ptr);
        std::swap(longest_order_ptr, shortest_order_ptr);
    }

    // Travel through price levels
    while ((longest_level_ptr != nullptr) && (shortest_level_ptr != nullptr))
    {
        // Travel through orders at current price levels
        while ((longest_order_ptr != nullptr) && (shortest_order_ptr != nullptr))
        {
            uint64_t need = required - available;
            uint64_t quantity = shortest_order_ptr->IsAON() ? shortest_order_ptr->Quantity : std::min(shortest_order_ptr->Quantity, need);
            available += quantity;

            // Matching is possible, return the chain size
            if (required == available)
                return required;

            // Swap longest and shortest chains
            if (required < available)
            {
                OrderNode* next = longest_order_ptr->next;
                longest_order_ptr = shortest_order_ptr;
                shortest_order_ptr = next;
                std::swap(required, available);
                continue;
            }

            // Take the next order
            shortest_order_ptr = shortest_order_ptr->next;
        }

        // Switch to the next longest price level
        if (longest_order_ptr == nullptr)
        {
            longest_level_ptr = order_book_ptr->GetNextLevel(longest_level_ptr);
            if (longest_level_ptr != nullptr)
                longest_order_ptr = longest_level_ptr->OrderList.front();
        }

        // Switch to the next shortest price level
        if (shortest_order_ptr == nullptr)
        {
            shortest_level_ptr = order_book_ptr->GetNextLevel(shortest_level_ptr);
            if (shortest_level_ptr != nullptr)
                shortest_order_ptr = shortest_level_ptr->OrderList.front();
        }
    }

    // Matching is not available
    return 0;
}

void MarketManager::ExecuteMatchingChain(OrderBook* order_book_ptr, LevelNode* level_ptr, uint64_t price, uint64_t volume)
{
    LevelNode* next_level_ptr;

    // Execute all orders in the matching chain
    while ((volume > 0) && (level_ptr != nullptr))
    {
        // Get the next prive level to execute
        next_level_ptr = order_book_ptr->GetNextLevel(level_ptr);

        // Execute all orders in the current price level
        while ((volume > 0) && !level_ptr->OrderList.empty())
        {
            // Find the order to execute
            OrderNode* executing_order_ptr = level_ptr->OrderList.front();

            uint64_t quantity;

            // Execute order
            if (executing_order_ptr->IsAON())
            {
                // Get the execution quantity
                quantity = executing_order_ptr->Quantity;

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

                // Delete the executing order from the order book
                DeleteOrder(executing_order_ptr->Id, true);
            }
            else
            {
                // Get the execution quantity
                quantity = std::min(executing_order_ptr->Quantity, volume);

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

                // Reduce the executing order in the order book
                ReduceOrder(executing_order_ptr->Id, quantity, true);
            }

            // Reduce the execution chain
            volume -= quantity;
        }

        // Switch to the next price level
        level_ptr = next_level_ptr;
    }
}

void MarketManager::UpdateLevel(const OrderBook& order_book, const LevelUpdate& update) const
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

} // namespace Matching
} // namespace CppTrader
