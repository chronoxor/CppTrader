/*!
    \file market_manager.cpp
    \brief Market manager implementation
    \author Ivan Shynkarenka
    \date 03.08.2017
    \copyright MIT License
*/

#include "trader/market_manager.h"

#include "errors/exceptions.h"
#include "string/format.h"

namespace CppTrader {

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

void MarketManager::AddSymbol(const Symbol& symbol)
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
        throwex CppCommon::RuntimeException("Duplicate symbol detected! Symbol Id = {}"_format(symbol.Id));
    }
    _symbols[symbol.Id] = symbol_ptr;

    // Call the corresponding handler
    _market_handler.onAddSymbol(*symbol_ptr);
}

void MarketManager::DeleteSymbol(uint32_t id)
{
    assert(((id < _symbols.size()) && (_symbols[id] != nullptr)) && "Symbol not found!");
    if ((_symbols.size() <= id) || (_symbols[id] == nullptr))
        throwex CppCommon::RuntimeException("Symbol not found! Symbol Id = {}"_format(id));

    // Get the symbol by Id
    Symbol* symbol_ptr = _symbols[id];

    // Call the corresponding handler
    _market_handler.onDeleteSymbol(*symbol_ptr);

    // Erase the symbol
    _symbols[id] = nullptr;

    // Release the symbol
    _symbol_pool.Release(symbol_ptr);
}

void MarketManager::AddOrderBook(const Symbol& symbol)
{
    assert(((symbol.Id < _symbols.size()) && (_symbols[symbol.Id] != nullptr)) && "Symbol not found!");
    if ((_symbols.size() <= symbol.Id) || (_symbols[symbol.Id] == nullptr))
        throwex CppCommon::RuntimeException("Symbol not found for a new order book! Symbol Id = {}"_format(symbol.Id));

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
        throwex CppCommon::RuntimeException("Duplicate order book detected! Order book symbol Id = {}"_format(symbol.Id));
    }
    _order_books[symbol.Id] = order_book_ptr;

    // Call the corresponding handler
    _market_handler.onAddOrderBook(*order_book_ptr);
}

void MarketManager::DeleteOrderBook(uint32_t id)
{
    assert(((id < _order_books.size()) && (_order_books[id] != nullptr)) && "Order book not found!");
    if ((_order_books.size() <= id) || (_order_books[id] == nullptr))
        throwex CppCommon::RuntimeException("Order book not found! Order book symbol Id = {}"_format(id));

    // Get the order book by Id
    OrderBook* order_book_ptr = _order_books[id];

    // Call the corresponding handler
    _market_handler.onDeleteOrderBook(*order_book_ptr);

    // Erase the order book
    _order_books[id] = nullptr;

    // Release the order book
    _order_book_pool.Release(order_book_ptr);
}

void MarketManager::AddOrder(const Order& order)
{
    // Validate parameters
    assert((order.Id > 0) && "Order Id must be greater than zero!");
    if (order.Id == 0)
        throwex CppCommon::ArgumentException("Order Id must be greater than zero!");
    assert((order.Quantity > 0) && "Order quantity must be greater than zero!");
    if (order.Quantity == 0)
        throwex CppCommon::ArgumentException("Order quantity must be greater than zero!");

    // Create a new order
    OrderNode* order_ptr = _order_pool.Create(order);

    // Insert the order
    if (!_orders.insert(std::make_pair(order.Id, order_ptr)).second)
    {
        // Release the order
        _order_pool.Release(order_ptr);
        throwex CppCommon::RuntimeException("Duplicate order detected! Order Id = {}"_format(order.Id));
    }

    // Call the corresponding handler
    _market_handler.onAddOrder(*order_ptr);

    // Get the valid order book for the new order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Add the new order into the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
    }
}

void MarketManager::ReduceOrder(uint64_t id, uint64_t quantity)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::ArgumentException("Order Id must be greater than zero!");
    assert((quantity > 0) && "Order quantity must be greater than zero!");
    if (quantity == 0)
        throwex CppCommon::ArgumentException("Order quantity must be greater than zero!");

    // Get the order to reduce
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Calculate the minimal possible order quantity to reduce
    quantity = std::min(quantity, order_ptr->Quantity);

    // Reduce the order quantity
    order_ptr->Quantity -= quantity;

    // Get the valid order book for the reducing order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Reduce the order in the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity));
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
}

void MarketManager::ModifyOrder(uint64_t id, uint64_t new_price, uint64_t new_quantity)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::ArgumentException("Order Id must be greater than zero!");

    // Get the order to modify
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the modifying order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Delete the order from the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));
    }

    // Modify the order
    order_ptr->Price = new_price;
    order_ptr->Quantity = new_quantity;

    // Update the order or delete the empty order
    if (order_ptr->Quantity > 0)
    {
        if (order_book_ptr != nullptr)
        {
            // Add the modified order into the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
        }

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
}

void MarketManager::ReplaceOrder(uint64_t id, uint64_t new_id, uint64_t new_price, uint64_t new_quantity)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::ArgumentException("Order Id must be greater than zero!");
    assert((new_id > 0) && "New order Id must be greater than zero!");
    if (new_id == 0)
        throwex CppCommon::ArgumentException("New order Id must be greater than zero!");

    // Get the order to replace
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the replacing order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Delete the old order from the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));
    }

    if (new_quantity > 0)
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        // Replace the order
        _orders.erase(order_it);
        order_ptr->Id = new_id;
        order_ptr->Price = new_price;
        order_ptr->Quantity = new_quantity;
        if (!_orders.insert(std::make_pair(new_id, order_ptr)).second)
        {
            // Release the order
            _order_pool.Release(order_ptr);
            throwex CppCommon::RuntimeException("Duplicate order detected! Order Id = {}"_format(new_id));
        }

        // Call the corresponding handler
        _market_handler.onAddOrder(*order_ptr);

        if (order_book_ptr != nullptr)
        {
            // Add the modified order into the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
        }
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
}

void MarketManager::ReplaceOrder(uint64_t id, const Order& new_order)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::ArgumentException("Order Id must be greater than zero!");
    assert((new_order.Id > 0) && "New order Id must be greater than zero!");
    if (new_order.Id == 0)
        throwex CppCommon::ArgumentException("New order Id must be greater than zero!");

    // Get the order to replace
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the replacing order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Delete the old order from the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));
    }

    if (new_order.Quantity > 0)
    {
        // Call the corresponding handler
        _market_handler.onDeleteOrder(*order_ptr);

        // Replace the order
        _orders.erase(order_it);
        *order_ptr = new_order;
        if (!_orders.insert(std::make_pair(new_order.Id, order_ptr)).second)
        {
            // Release the order
            _order_pool.Release(order_ptr);
            throwex CppCommon::RuntimeException("Duplicate order detected! Order Id = {}"_format(new_order.Id));
        }

        // Call the corresponding handler
        _market_handler.onAddOrder(*order_ptr);

        // Get the valid order book for the new order
        order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
        if (order_book_ptr != nullptr)
        {
            // Add the modified order into the order book
            UpdateLevel(*order_book_ptr, order_book_ptr->AddOrder(order_ptr));
        }
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
}

void MarketManager::DeleteOrder(uint64_t id)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::ArgumentException("Order Id must be greater than zero!");

    // Get the order to delete
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Get the valid order book for the deleting order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Delete the order from the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->DeleteOrder(order_ptr));
    }

    // Call the corresponding handler
    _market_handler.onDeleteOrder(*order_ptr);

    // Erase the order
    _orders.erase(order_it);

    // Relase the order
    _order_pool.Release(order_ptr);
}

void MarketManager::ExecuteOrder(uint64_t id, uint64_t quantity)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::ArgumentException("Order Id must be greater than zero!");
    assert((quantity > 0) && "Order quantity must be greater than zero!");
    if (quantity == 0)
        throwex CppCommon::ArgumentException("Order quantity must be greater than zero!");

    // Get the order to execute
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Calculate the minimal possible order quantity to execute
    quantity = std::min(quantity, order_ptr->Quantity);

    // Call the corresponding handler
    _market_handler.onExecuteOrder(*order_ptr, order_ptr->Price, quantity);

    // Reduce the order quantity
    order_ptr->Quantity -= quantity;

    // Get the valid order book for the executing order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Reduce the order in the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity));
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
}

void MarketManager::ExecuteOrder(uint64_t id, uint64_t price, uint64_t quantity)
{
    // Validate parameters
    assert((id > 0) && "Order Id must be greater than zero!");
    if (id == 0)
        throwex CppCommon::ArgumentException("Order Id must be greater than zero!");
    assert((quantity > 0) && "Order quantity must be greater than zero!");
    if (quantity == 0)
        throwex CppCommon::ArgumentException("Order quantity must be greater than zero!");

    // Get the order to execute
    auto order_it = _orders.find(id);
    assert((order_it != _orders.end()) && "Order not found!");
    if (order_it == _orders.end())
        throwex CppCommon::RuntimeException("Order not found! Order Id = {}"_format(id));
    OrderNode* order_ptr = (OrderNode*)order_it->second;

    // Calculate the minimal possible order quantity to execute
    quantity = std::min(quantity, order_ptr->Quantity);

    // Call the corresponding handler
    _market_handler.onExecuteOrder(*order_ptr, price, quantity);

    // Reduce the order quantity
    order_ptr->Quantity -= quantity;

    // Get the valid order book for the executing order
    OrderBook* order_book_ptr = (OrderBook*)GetOrderBook(order_ptr->SymbolId);
    if (order_book_ptr != nullptr)
    {
        // Reduce the order in the order book
        UpdateLevel(*order_book_ptr, order_book_ptr->ReduceOrder(order_ptr, quantity));
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

} // namespace CppTrader
