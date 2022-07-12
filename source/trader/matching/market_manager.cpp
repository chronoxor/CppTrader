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
    for (const auto& order : _orders)
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
    OrderBook* order_book_ptr = _order_book_pool.Create(*this, *symbol_ptr);

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
        case OrderType::TRAILING_STOP:
            return AddStopOrder(order, false);
        case OrderType::STOP_LIMIT:
        case OrderType::TRAILING_STOP_LIMIT:
            return AddStopLimitOrder(order, false);
        default:
            return ErrorCode::ORDER_TYPE_INVALID;
    }
}

ErrorCode MarketManager::AddMarketOrder(const Order& order, bool recursive)
{
    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    // Call the corresponding handler
    _market_handler.onAddOrder(new_order);

    // Automatic order matching
    if (_matching && !recursive)
        MatchMarket(order_book_ptr, &new_order);

    // Call the corresponding handler
    _market_handler.onDeleteOrder(new_order);

    // Automatic order matching
    if (_matching && !recursive)
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddLimitOrder(const Order& order, bool recursive)
{
    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    // Call the corresponding handler
    _market_handler.onAddOrder(new_order);

    // Automatic order matching
    if (_matching && !recursive)
        MatchLimit(order_book_ptr, &new_order);

    // Add a new order or delete remaining part in case of 'Immediate-Or-Cancel'/'Fill-Or-Kill' order
    if ((new_order.LeavesQuantity > 0) && !new_order.IsIOC() && !new_order.IsFOK())
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
    if (_matching && !recursive)
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddStopOrder(const Order& order, bool recursive)
{
    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    // Recalculate stop price for trailing stop orders
    if (new_order.IsTrailingStop() || new_order.IsTrailingStopLimit())
        new_order.StopPrice = order_book_ptr->CalculateTrailingStopPrice(new_order);

    // Call the corresponding handler
    _market_handler.onAddOrder(new_order);

    // Automatic order matching
    if (_matching && !recursive)
    {
        // Find the price to match the stop order
        uint64_t stop_price = new_order.IsBuy() ? order_book_ptr->GetMarketPriceAsk() : order_book_ptr->GetMarketPriceBid();

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
            if (_matching && !recursive)
                Match(order_book_ptr);

            // Reset matching price
            order_book_ptr->ResetMatchingPrice();

            return ErrorCode::OK;
        }
    }

    // Add a new order
    if (new_order.LeavesQuantity > 0)
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
        if (order_ptr->IsTrailingStop() || order_ptr->IsTrailingStopLimit())
            order_book_ptr->AddTrailingStopOrder(order_ptr);
        else
            order_book_ptr->AddStopOrder(order_ptr);
    }
    else
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(new_order);
    }

    // Automatic order matching
    if (_matching && !recursive)
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::AddStopLimitOrder(const Order& order, bool recursive)
{
    // Get the valid order book for the order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order.SymbolId);
    if (order_book_ptr == nullptr)
        return ErrorCode::ORDER_BOOK_NOT_FOUND;

    Order new_order(order);

    // Recalculate stop price for trailing stop orders
    if (new_order.IsTrailingStop() || new_order.IsTrailingStopLimit())
    {
        int64_t diff = new_order.Price - new_order.StopPrice;
        new_order.StopPrice = order_book_ptr->CalculateTrailingStopPrice(new_order);
        new_order.Price = new_order.StopPrice + diff;
    }

    // Call the corresponding handler
    _market_handler.onAddOrder(new_order);

    // Automatic order matching
    if (_matching && !recursive)
    {
        // Find the price to match the stop-limit order
        uint64_t stop_price = new_order.IsBuy() ? order_book_ptr->GetMarketPriceAsk() : order_book_ptr->GetMarketPriceBid();

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
            if ((new_order.LeavesQuantity > 0) && !new_order.IsIOC() && !new_order.IsFOK())
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
            if (_matching && !recursive)
                Match(order_book_ptr);

            // Reset matching price
            order_book_ptr->ResetMatchingPrice();

            return ErrorCode::OK;
        }
    }

    // Add a new order
    if (new_order.LeavesQuantity > 0)
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
        if (order_ptr->IsTrailingStop() || order_ptr->IsTrailingStopLimit())
            order_book_ptr->AddTrailingStopOrder(order_ptr);
        else
            order_book_ptr->AddStopOrder(order_ptr);
    }
    else
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(new_order);
    }

    // Automatic order matching
    if (_matching && !recursive)
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::ReduceOrder(uint64_t id, uint64_t quantity)
{
    return ReduceOrder(id, quantity, false);
}

ErrorCode MarketManager::ReduceOrder(uint64_t id, uint64_t quantity, bool recursive)
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
    quantity = std::min(quantity, order_ptr->LeavesQuantity);

    uint64_t hidden = order_ptr->HiddenQuantity();
    uint64_t visible = order_ptr->VisibleQuantity();

    // Reduce the order leaves quantity
    order_ptr->LeavesQuantity -= quantity;

    hidden -= order_ptr->HiddenQuantity();
    visible -= order_ptr->VisibleQuantity();

    // Update the order or delete the empty order
    if (order_ptr->LeavesQuantity > 0)
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
    if (_matching && !recursive)
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity)
{
    return ModifyOrder(id, new_price, new_quantity, false, false);
}

ErrorCode MarketManager::MitigateOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity)
{
    return ModifyOrder(id, new_price, new_quantity, true, false);
}

ErrorCode MarketManager::ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity, bool mitigate, bool recursive)
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
    order_ptr->LeavesQuantity = new_quantity;

    // In-Flight Mitigation (IFM)
    if (mitigate)
    {
        // This calculation has the goal of preventing orders from being overfilled
        if (new_quantity > order_ptr->ExecutedQuantity)
            order_ptr->LeavesQuantity = new_quantity - order_ptr->ExecutedQuantity;
        else
            order_ptr->LeavesQuantity = 0;
    }

    // Update the order
    if (order_ptr->LeavesQuantity > 0)
    {
        // Call the corresponding handler
        _market_handler.onUpdateOrder(*order_ptr);

        // Automatic order matching
        if (_matching && !recursive)
            MatchLimit(order_book_ptr, order_ptr);

        // Add non empty order into the order book
        if (order_ptr->LeavesQuantity > 0)
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
    if (order_ptr->LeavesQuantity == 0)
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        // Erase the order
        _orders.erase(order_it);

        // Relase the order
        _order_pool.Release(order_ptr);
    }

    // Automatic order matching
    if (_matching && !recursive)
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

ErrorCode MarketManager::ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity)
{
    return ReplaceOrder(id, new_id, new_price, new_quantity, false);
}

ErrorCode MarketManager::ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity, bool recursive)
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
    order_ptr->ExecutedQuantity = 0;
    order_ptr->LeavesQuantity = new_quantity;

    // Call the corresponding handler
    _market_handler.onAddOrder(*order_ptr);

    // Automatic order matching
    if (_matching && !recursive)
        MatchLimit(order_book_ptr, order_ptr);

    if (order_ptr->LeavesQuantity > 0)
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
    if (_matching && !recursive)
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

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

ErrorCode MarketManager::DeleteOrder(uint64_t id, bool recursive)
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
    if (_matching && !recursive)
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

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
    quantity = std::min(quantity, order_ptr->LeavesQuantity);

    // Call the corresponding handler
    _market_handler.onExecuteOrder(*order_ptr, order_ptr->Price, quantity);

    // Update the corresponding market price
    order_book_ptr->UpdateLastPrice(*order_ptr, order_ptr->Price);
    order_book_ptr->UpdateMatchingPrice(*order_ptr, order_ptr->Price);

    uint64_t hidden = order_ptr->HiddenQuantity();
    uint64_t visible = order_ptr->VisibleQuantity();

    // Increase the order executed quantity
    order_ptr->ExecutedQuantity += quantity;

    // Reduce the order leaves quantity
    order_ptr->LeavesQuantity -= quantity;

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
    if (order_ptr->LeavesQuantity > 0)
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
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

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
    quantity = std::min(quantity, order_ptr->LeavesQuantity);

    // Call the corresponding handler
    _market_handler.onExecuteOrder(*order_ptr, price, quantity);

    // Update the corresponding market price
    order_book_ptr->UpdateLastPrice(*order_ptr, price);
    order_book_ptr->UpdateMatchingPrice(*order_ptr, price);

    uint64_t hidden = order_ptr->HiddenQuantity();
    uint64_t visible = order_ptr->VisibleQuantity();

    // Increase the order executed quantity
    order_ptr->ExecutedQuantity += quantity;

    // Reduce the order leaves quantity
    order_ptr->LeavesQuantity -= quantity;

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
    if (order_ptr->LeavesQuantity > 0)
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
        Match(order_book_ptr);

    // Reset matching price
    order_book_ptr->ResetMatchingPrice();

    return ErrorCode::OK;
}

void MarketManager::Match()
{
    for (auto order_book_ptr : _order_books)
        if (order_book_ptr != nullptr)
            Match(order_book_ptr);
}

void MarketManager::Match(OrderBook* order_book_ptr)
{
    // Matching loop
    for (;;)
    {
        // Check the arbitrage bid/ask prices
        while ((order_book_ptr->_best_bid != nullptr) &&
               (order_book_ptr->_best_ask != nullptr) &&
               (order_book_ptr->_best_bid->Price >= order_book_ptr->_best_ask->Price))
        {
            // Find the best bid/ask price level
            LevelNode* bid_level_ptr = order_book_ptr->_best_bid;
            LevelNode* ask_level_ptr = order_book_ptr->_best_ask;

            // Find the first order to execute and the first order to reduce
            OrderNode* bid_order_ptr = bid_level_ptr->OrderList.front();
            OrderNode* ask_order_ptr = ask_level_ptr->OrderList.front();

            // Execute crossed orders
            while ((bid_order_ptr != nullptr) && (ask_order_ptr != nullptr))
            {
                // Find the next orders pair
                OrderNode* next_bid_order_ptr = bid_order_ptr->next;
                OrderNode* next_ask_order_ptr = ask_order_ptr->next;

                // Special case for 'All-Or-None' orders
                if (bid_order_ptr->IsAON() || ask_order_ptr->IsAON())
                {
                    // Calculate the matching chain
                    uint64_t chain = CalculateMatchingChain(order_book_ptr, bid_level_ptr, ask_level_ptr);

                    // Matching is not avaliable
                    if (chain == 0)
                        return;

                    // Execute orders in the matching chain
                    if (bid_order_ptr->IsAON())
                    {
                        uint64_t price = bid_order_ptr->Price;
                        ExecuteMatchingChain(order_book_ptr, bid_level_ptr, price, chain);
                        ExecuteMatchingChain(order_book_ptr, ask_level_ptr, price, chain);
                    }
                    else
                    {
                        uint64_t price = ask_order_ptr->Price;
                        ExecuteMatchingChain(order_book_ptr, ask_level_ptr, price, chain);
                        ExecuteMatchingChain(order_book_ptr, bid_level_ptr, price, chain);
                    }

                    break;
                }

                // Find the best order to execute and the best order to reduce
                OrderNode* executing_order_ptr = bid_order_ptr;
                OrderNode* reducing_order_ptr = ask_order_ptr;
                if (executing_order_ptr->LeavesQuantity > reducing_order_ptr->LeavesQuantity)
                    std::swap(executing_order_ptr, reducing_order_ptr);

                // Get the execution quantity
                uint64_t quantity = executing_order_ptr->LeavesQuantity;

                // Get the execution price
                uint64_t price = executing_order_ptr->Price;

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

                // Update the corresponding market price
                order_book_ptr->UpdateLastPrice(*executing_order_ptr, price);
                order_book_ptr->UpdateMatchingPrice(*executing_order_ptr, price);

                // Increase the order executed quantity
                executing_order_ptr->ExecutedQuantity += quantity;

                // Delete the executing order from the order book
                DeleteOrder(executing_order_ptr->Id, true);

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*reducing_order_ptr, price, quantity);

                // Update the corresponding market price
                order_book_ptr->UpdateLastPrice(*reducing_order_ptr, price);
                order_book_ptr->UpdateMatchingPrice(*reducing_order_ptr, price);

                // Increase the order executed quantity
                reducing_order_ptr->ExecutedQuantity += quantity;

                // Reduce the remaining order in the order book
                ReduceOrder(reducing_order_ptr->Id, quantity, true);

                // Move to the next orders pair at the same price level
                bid_order_ptr = next_bid_order_ptr;
                ask_order_ptr = next_ask_order_ptr;
            }

            // Activate stop orders only if the current price level changed
            ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_buy_stop(), order_book_ptr->GetMarketPriceAsk());
            ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_sell_stop(), order_book_ptr->GetMarketPriceBid());
        }

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
            uint64_t chain = CalculateMatchingChain(order_book_ptr, level_ptr, order_ptr->Price, order_ptr->LeavesQuantity);

            // Matching is not avaliable
            if (chain == 0)
                return;

            // Execute orders in the matching chain
            ExecuteMatchingChain(order_book_ptr, level_ptr, order_ptr->Price, chain);

            // Call the corresponding handler
            _market_handler.onExecuteOrder(*order_ptr, order_ptr->Price, order_ptr->LeavesQuantity);

            // Update the corresponding market price
            order_book_ptr->UpdateLastPrice(*order_ptr, order_ptr->Price);
            order_book_ptr->UpdateMatchingPrice(*order_ptr, order_ptr->Price);

            // Increase the order executed quantity
            order_ptr->ExecutedQuantity += order_ptr->LeavesQuantity;

            // Reduce the order leaves quantity
            order_ptr->LeavesQuantity = 0;

            return;
        }

        // Find the first order to execute
        OrderNode* executing_order_ptr = level_ptr->OrderList.front();

        // Execute crossed orders
        while (executing_order_ptr != nullptr)
        {
            // Find the next order to execute
            OrderNode* next_executing_order_ptr = executing_order_ptr->next;

            // Get the execution quantity
            uint64_t quantity = std::min(executing_order_ptr->LeavesQuantity, order_ptr->LeavesQuantity);

            // Special case for 'All-Or-None' orders
            if (executing_order_ptr->IsAON() && (executing_order_ptr->LeavesQuantity > order_ptr->LeavesQuantity))
                return;

            // Get the execution price
            uint64_t price = executing_order_ptr->Price;

            // Call the corresponding handler
            _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

            // Update the corresponding market price
            order_book_ptr->UpdateLastPrice(*executing_order_ptr, price);
            order_book_ptr->UpdateMatchingPrice(*executing_order_ptr, price);

            // Increase the order executed quantity
            executing_order_ptr->ExecutedQuantity += quantity;

            // Reduce the executing order in the order book
            ReduceOrder(executing_order_ptr->Id, quantity, true);

            // Call the corresponding handler
            _market_handler.onExecuteOrder(*order_ptr, price, quantity);

            // Update the corresponding market price
            order_book_ptr->UpdateLastPrice(*order_ptr, price);
            order_book_ptr->UpdateMatchingPrice(*order_ptr, price);

            // Increase the order executed quantity
            order_ptr->ExecutedQuantity += quantity;

            // Reduce the order leaves quantity
            order_ptr->LeavesQuantity -= quantity;
            if (order_ptr->LeavesQuantity == 0)
                return;

            // Move to the next order to execute at the same price level
            executing_order_ptr = next_executing_order_ptr;
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

        // Try to activate buy stop orders
        if (ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_buy_stop(), order_book_ptr->GetMarketPriceAsk()) ||
            ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_trailing_buy_stop(), order_book_ptr->GetMarketPriceAsk()))
        {
            result = true;
            stop = false;
        }

        // Recalculate trailing buy stop orders
        RecalculateTrailingStopPrice(order_book_ptr, order_book_ptr->_best_ask);

        // Try to activate sell stop orders
        if (ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_sell_stop(), order_book_ptr->GetMarketPriceBid()) ||
            ActivateStopOrders(order_book_ptr, (LevelNode*)order_book_ptr->best_trailing_sell_stop(), order_book_ptr->GetMarketPriceBid()))
        {
            result = true;
            stop = false;
        }

        // Recalculate trailing sell stop orders
        RecalculateTrailingStopPrice(order_book_ptr, order_book_ptr->_best_bid);
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

        // Find the stop order to activate
        OrderNode* activating_order_ptr = level_ptr->OrderList.front();

        // Activate all stop orders
        while (activating_order_ptr != nullptr)
        {
            // Find the next order to activate
            OrderNode* next_activating_order_ptr = activating_order_ptr->next;

            // Activate the stop order
            switch (activating_order_ptr->Type)
            {
                case OrderType::STOP:
                case OrderType::TRAILING_STOP:
                    result = ActivateStopOrder(order_book_ptr, activating_order_ptr);
                    break;
                case OrderType::STOP_LIMIT:
                case OrderType::TRAILING_STOP_LIMIT:
                    result = ActivateStopLimitOrder(order_book_ptr, activating_order_ptr);
                    break;
                default:
                    assert(false && "Unsupported order type!");
                    break;

            }

            // Move to the next order to activate at the same price level
            activating_order_ptr = next_activating_order_ptr;
        }
    }

    return result;
}

bool MarketManager::ActivateStopOrder(OrderBook* order_book_ptr, OrderNode* order_ptr)
{
    // Delete the stop order from the order book
    if (order_ptr->IsTrailingStop() || order_ptr->IsTrailingStopLimit())
        order_book_ptr->DeleteTrailingStopOrder(order_ptr);
    else
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
    if (order_ptr->IsTrailingStop() || order_ptr->IsTrailingStopLimit())
        order_book_ptr->DeleteTrailingStopOrder(order_ptr);
    else
        order_book_ptr->DeleteStopOrder(order_ptr);

    // Convert the stop-limit order into the limit order
    order_ptr->Type = OrderType::LIMIT;
    order_ptr->StopPrice = 0;

    // Call the corresponding handler
    _market_handler.onUpdateOrder(*order_ptr);

    // Match the limit order
    MatchLimit(order_book_ptr, order_ptr);

    // Add a new limit order or delete remaining part in case of 'Immediate-Or-Cancel'/'Fill-Or-Kill' order
    if ((order_ptr->LeavesQuantity > 0) && !order_ptr->IsIOC() && !order_ptr->IsFOK())
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
            uint64_t quantity = order_ptr->IsAON() ? order_ptr->LeavesQuantity : std::min(order_ptr->LeavesQuantity, need);
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
    uint64_t required = longest_order_ptr->LeavesQuantity;
    uint64_t available = 0;

    // Find the initial longest order chain
    if (longest_order_ptr->IsAON() && shortest_order_ptr->IsAON())
    {
        // Choose the longest 'All-Or-None' order
        if (shortest_order_ptr->LeavesQuantity > longest_order_ptr->LeavesQuantity)
        {
            required = shortest_order_ptr->LeavesQuantity;
            available = 0;
            std::swap(longest_level_ptr, shortest_level_ptr);
            std::swap(longest_order_ptr, shortest_order_ptr);
        }
    }
    else if (shortest_order_ptr->IsAON())
    {
        required = shortest_order_ptr->LeavesQuantity;
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
            uint64_t quantity = shortest_order_ptr->IsAON() ? shortest_order_ptr->LeavesQuantity : std::min(shortest_order_ptr->LeavesQuantity, need);
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
    // Execute all orders in the matching chain
    while ((volume > 0) && (level_ptr != nullptr))
    {
        // Get the next prive level to execute
        LevelNode* next_level_ptr = order_book_ptr->GetNextLevel(level_ptr);

        // Find the first order to execute
        OrderNode* executing_order_ptr = level_ptr->OrderList.front();

        // Execute all orders in the current price level
        while ((volume > 0) && (executing_order_ptr != nullptr))
        {
            // Find the next order to execute
            OrderNode* next_executing_order_ptr = executing_order_ptr->next;

            uint64_t quantity;

            // Execute order
            if (executing_order_ptr->IsAON())
            {
                // Get the execution quantity
                quantity = executing_order_ptr->LeavesQuantity;

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

                // Update the corresponding market price
                order_book_ptr->UpdateLastPrice(*executing_order_ptr, price);
                order_book_ptr->UpdateMatchingPrice(*executing_order_ptr, price);

                // Increase the order executed quantity
                executing_order_ptr->ExecutedQuantity += quantity;

                // Delete the executing order from the order book
                DeleteOrder(executing_order_ptr->Id, true);
            }
            else
            {
                // Get the execution quantity
                quantity = std::min(executing_order_ptr->LeavesQuantity, volume);

                // Call the corresponding handler
                _market_handler.onExecuteOrder(*executing_order_ptr, price, quantity);

                // Update the corresponding market price
                order_book_ptr->UpdateLastPrice(*executing_order_ptr, price);
                order_book_ptr->UpdateMatchingPrice(*executing_order_ptr, price);

                // Increase the order executed quantity
                executing_order_ptr->ExecutedQuantity += quantity;

                // Reduce the executing order in the order book
                ReduceOrder(executing_order_ptr->Id, quantity, true);
            }

            // Reduce the execution chain
            volume -= quantity;

            // Move to the next order to execute at the same price level
            executing_order_ptr = next_executing_order_ptr;
        }

        // Move to the next price level
        level_ptr = next_level_ptr;
    }
}

void MarketManager::RecalculateTrailingStopPrice(OrderBook* order_book_ptr, LevelNode* level_ptr)
{
    if (level_ptr == nullptr)
        return;

    uint64_t new_trailing_price;

    // Check if we should skip the recalculation because of the market price goes to the wrong direction
    if (level_ptr->Type == LevelType::ASK)
    {
        uint64_t old_trailing_price = order_book_ptr->_trailing_ask_price;
        new_trailing_price = order_book_ptr->GetMarketTrailingStopPriceAsk();
        order_book_ptr->_trailing_ask_price = new_trailing_price;
        if (new_trailing_price >= old_trailing_price)
            return;
    }
    if (level_ptr->Type == LevelType::BID)
    {
        uint64_t old_trailing_price = order_book_ptr->_trailing_bid_price;
        new_trailing_price = order_book_ptr->GetMarketTrailingStopPriceBid();
        order_book_ptr->_trailing_bid_price = new_trailing_price;
        if (new_trailing_price <= old_trailing_price)
            return;
    }

    // Recalculate trailing stop orders
    LevelNode* previous = nullptr;
    LevelNode* current = (level_ptr->Type == LevelType::ASK) ? order_book_ptr->_best_trailing_buy_stop : order_book_ptr->_best_trailing_sell_stop;
    while (current != nullptr)
    {
        bool recalculated = false;

        // Find the first order to recalculate
        OrderNode* order_ptr = current->OrderList.front();

        while (order_ptr != nullptr)
        {
            // Find the next order to recalculate
            OrderNode* next_order_ptr = order_ptr->next;

            uint64_t old_stop_price = order_ptr->StopPrice;
            uint64_t new_stop_price = order_book_ptr->CalculateTrailingStopPrice(*order_ptr);

            // Trailing distance for the order must be changed
            if (new_stop_price != old_stop_price)
            {
                // Delete the order from the order book
                order_book_ptr->DeleteTrailingStopOrder(order_ptr);

                // Update the stop order price
                switch (order_ptr->Type)
                {
                    case OrderType::TRAILING_STOP:
                        order_ptr->StopPrice = new_stop_price;
                        break;
                    case OrderType::TRAILING_STOP_LIMIT:
                    {
                        int64_t diff = order_ptr->Price - order_ptr->StopPrice;
                        order_ptr->StopPrice = new_stop_price;
                        order_ptr->Price = order_ptr->StopPrice + diff;
                        break;
                    }
                    default:
                        assert(false && "Unsupported order type!");
                        break;

                }

                // Call the corresponding handler
                _market_handler.onUpdateOrder(*order_ptr);

                // Add the new stop order into the order book
                order_book_ptr->AddTrailingStopOrder(order_ptr);

                recalculated = true;
            }

            // Move to the next order to recalculate at the same price level
            order_ptr = next_order_ptr;
        }

        if (recalculated)
        {
            // Back to the previous stop price level
            current = (previous != nullptr) ? previous : ((level_ptr->Type == LevelType::ASK) ? order_book_ptr->_best_trailing_buy_stop : order_book_ptr->_best_trailing_sell_stop);
        }
        else
        {
            // Move to the next stop price level
            previous = current;
            current = order_book_ptr->GetNextTrailingStopLevel(current);
        }
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
