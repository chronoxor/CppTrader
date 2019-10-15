//
// Created by Ivan Shynkarenka on 15.08.2017
//

#include "test.h"

#include "trader/matching/market_manager.h"

using namespace CppCommon;
using namespace CppTrader::Matching;

namespace {

std::pair<int, int> BookOrders(const OrderBook* order_book_ptr)
{
    if (order_book_ptr == nullptr)
        return std::make_pair(0, 0);

    int bid_orders = 0;
    for (const auto& bid : order_book_ptr->bids())
        bid_orders += (int)bid.Orders;

    int ask_orders = 0;
    for (const auto& ask : order_book_ptr->asks())
        ask_orders += (int)ask.Orders;

    return std::make_pair(bid_orders, ask_orders);
}

std::pair<int, int> BookVolume(const OrderBook* order_book_ptr)
{
    if (order_book_ptr == nullptr)
        return std::make_pair(0, 0);

    int bid_volume = 0;
    for (const auto& bid : order_book_ptr->bids())
        bid_volume += (int)bid.TotalVolume;

    int ask_volume = 0;
    for (const auto& ask : order_book_ptr->asks())
        ask_volume += (int)ask.TotalVolume;

    return std::make_pair(bid_volume, ask_volume);
}

std::pair<int, int> BookVisibleVolume(const OrderBook* order_book_ptr)
{
    if (order_book_ptr == nullptr)
        return std::make_pair(0, 0);

    int bid_volume = 0;
    for (const auto& bid : order_book_ptr->bids())
        bid_volume += (int)bid.VisibleVolume;

    int ask_volume = 0;
    for (const auto& ask : order_book_ptr->asks())
        ask_volume += (int)ask.VisibleVolume;

    return std::make_pair(bid_volume, ask_volume);
}

std::pair<int, int> BookStopOrders(const OrderBook* order_book_ptr)
{
    if (order_book_ptr == nullptr)
        return std::make_pair(0, 0);

    int buy_orders = 0;
    for (const auto& buy : order_book_ptr->buy_stop())
        buy_orders += (int)buy.Orders;
    for (const auto& buy : order_book_ptr->trailing_buy_stop())
        buy_orders += (int)buy.Orders;

    int sell_orders = 0;
    for (const auto& sell : order_book_ptr->sell_stop())
        sell_orders += (int)sell.Orders;
    for (const auto& sell : order_book_ptr->trailing_sell_stop())
        sell_orders += (int)sell.Orders;

    return std::make_pair(buy_orders, sell_orders);
}

std::pair<int, int> BookStopVolume(const OrderBook* order_book_ptr)
{
    if (order_book_ptr == nullptr)
        return std::make_pair(0, 0);

    int buy_volume = 0;
    for (const auto& buy : order_book_ptr->buy_stop())
        buy_volume += (int)buy.TotalVolume;
    for (const auto& buy : order_book_ptr->trailing_buy_stop())
        buy_volume += (int)buy.TotalVolume;

    int sell_volume = 0;
    for (const auto& sell : order_book_ptr->sell_stop())
        sell_volume += (int)sell.TotalVolume;
    for (const auto& sell : order_book_ptr->trailing_sell_stop())
        sell_volume += (int)sell.TotalVolume;

    return std::make_pair(buy_volume, sell_volume);
}

}

TEST_CASE("Automatic matching - market order", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add buy limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10));
    market.AddOrder(Order::BuyLimit(2, 0, 10, 20));
    market.AddOrder(Order::BuyLimit(3, 0, 10, 30));
    market.AddOrder(Order::BuyLimit(4, 0, 20, 10));
    market.AddOrder(Order::BuyLimit(5, 0, 20, 20));
    market.AddOrder(Order::BuyLimit(6, 0, 20, 30));
    market.AddOrder(Order::BuyLimit(7, 0, 30, 10));
    market.AddOrder(Order::BuyLimit(8, 0, 30, 20));
    market.AddOrder(Order::BuyLimit(9, 0, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(9, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(180, 0));

    // Add sell limit orders
    market.AddOrder(Order::SellLimit(10, 0, 40, 30));
    market.AddOrder(Order::SellLimit(11, 0, 40, 20));
    market.AddOrder(Order::SellLimit(12, 0, 40, 10));
    market.AddOrder(Order::SellLimit(13, 0, 50, 30));
    market.AddOrder(Order::SellLimit(14, 0, 50, 20));
    market.AddOrder(Order::SellLimit(15, 0, 50, 10));
    market.AddOrder(Order::SellLimit(16, 0, 60, 30));
    market.AddOrder(Order::SellLimit(17, 0, 60, 20));
    market.AddOrder(Order::SellLimit(18, 0, 60, 10));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(9, 9));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(180, 180));

    // Automatic matching on add market order
    market.AddOrder(Order::SellMarket(19, 0, 15));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(8, 9));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(165, 180));

    // Automatic matching on add market order with slippage
    market.AddOrder(Order::SellMarket(20, 0, 100, 0));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(6, 9));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(120, 180));
    market.AddOrder(Order::BuyMarket(21, 0, 160, 20));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(6, 2));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(120, 20));

    // Automatic matching on add market order with reaching end of the book
    market.AddOrder(Order::SellMarket(22, 0, 1000));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 2));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 20));
    market.AddOrder(Order::BuyMarket(23, 0, 1000));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - limit order", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add buy limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10));
    market.AddOrder(Order::BuyLimit(2, 0, 10, 20));
    market.AddOrder(Order::BuyLimit(3, 0, 10, 30));
    market.AddOrder(Order::BuyLimit(4, 0, 20, 10));
    market.AddOrder(Order::BuyLimit(5, 0, 20, 20));
    market.AddOrder(Order::BuyLimit(6, 0, 20, 30));
    market.AddOrder(Order::BuyLimit(7, 0, 30, 10));
    market.AddOrder(Order::BuyLimit(8, 0, 30, 20));
    market.AddOrder(Order::BuyLimit(9, 0, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(9, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(180, 0));

    // Add sell limit orders
    market.AddOrder(Order::SellLimit(10, 0, 40, 30));
    market.AddOrder(Order::SellLimit(11, 0, 40, 20));
    market.AddOrder(Order::SellLimit(12, 0, 40, 10));
    market.AddOrder(Order::SellLimit(13, 0, 50, 30));
    market.AddOrder(Order::SellLimit(14, 0, 50, 20));
    market.AddOrder(Order::SellLimit(15, 0, 50, 10));
    market.AddOrder(Order::SellLimit(16, 0, 60, 30));
    market.AddOrder(Order::SellLimit(17, 0, 60, 20));
    market.AddOrder(Order::SellLimit(18, 0, 60, 10));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(9, 9));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(180, 180));

    // Automatic matching on add limit orders
    market.AddOrder(Order::SellLimit(19, 0, 30, 5));
    market.AddOrder(Order::SellLimit(20, 0, 30, 25));
    market.AddOrder(Order::SellLimit(21, 0, 30, 15));
    market.AddOrder(Order::SellLimit(22, 0, 30, 20));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(6, 10));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(120, 185));

    // Automatic matching on several levels
    market.AddOrder(Order::BuyLimit(23, 0, 60, 105));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(6, 5));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(120, 80));

    // Automatic matching on modify order
    market.ModifyOrder(15, 20, 20);
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(5, 4));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(100, 70));

    // Automatic matching on replace order
    market.ReplaceOrder(2, 24, 70, 100);
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(5, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(110, 0));
    market.ReplaceOrder(1, Order::SellLimit(25, 0, 0, 100));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - 'Immediate-Or-Cancel' limit order", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10));
    market.AddOrder(Order::BuyLimit(2, 0, 20, 20));
    market.AddOrder(Order::BuyLimit(3, 0, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(60, 0));

    // Automatic matching 'Immediate-Or-Cancel' order
    market.AddOrder(Order::SellLimit(4, 0, 10, 100, OrderTimeInForce::IOC));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - 'Fill-Or-Kill' limit order (filled)", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10));
    market.AddOrder(Order::BuyLimit(2, 0, 20, 20));
    market.AddOrder(Order::BuyLimit(3, 0, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(60, 0));

    // Automatic matching 'Fill-Or-Kill' order
    market.AddOrder(Order::SellLimit(4, 0, 10, 40, OrderTimeInForce::FOK));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(2, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(20, 0));
}

TEST_CASE("Automatic matching - 'Fill-Or-Kill' limit order (killed)", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10));
    market.AddOrder(Order::BuyLimit(2, 0, 20, 20));
    market.AddOrder(Order::BuyLimit(3, 0, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(60, 0));

    // Automatic matching 'Fill-Or-Kill' order
    market.AddOrder(Order::SellLimit(4, 0, 10, 100, OrderTimeInForce::FOK));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(60, 0));
}

TEST_CASE("Automatic matching - 'All-Or-None' limit order several levels full matching", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 20, 30, OrderTimeInForce::AON));
    market.AddOrder(Order::BuyLimit(2, 0, 20, 10));
    market.AddOrder(Order::BuyLimit(3, 0, 30, 30, OrderTimeInForce::AON));
    market.AddOrder(Order::BuyLimit(4, 0, 30, 10));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(4, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(80, 0));

    // Automatic matching 'All-Or-None' order
    market.AddOrder(Order::SellLimit(5, 0, 20, 80, OrderTimeInForce::AON));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - 'All-Or-None' limit order several levels partial matching", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 20, 30, OrderTimeInForce::AON));
    market.AddOrder(Order::BuyLimit(2, 0, 20, 10));
    market.AddOrder(Order::BuyLimit(3, 0, 30, 30, OrderTimeInForce::AON));
    market.AddOrder(Order::BuyLimit(4, 0, 30, 10));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(4, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(80, 0));

    // Place huge 'All-Or-None' order in the order book with arbitrage price
    market.AddOrder(Order::SellLimit(5, 0, 20, 100, OrderTimeInForce::AON));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(4, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(80, 100));

    // Automatic matching 'All-Or-None' order
    market.AddOrder(Order::BuyLimit(6, 0, 20, 20, OrderTimeInForce::AON));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - 'All-Or-None' limit order complex matching", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 20, OrderTimeInForce::AON));
    market.AddOrder(Order::SellLimit(2, 0, 10, 10, OrderTimeInForce::AON));
    market.AddOrder(Order::SellLimit(3, 0, 10, 5));
    market.AddOrder(Order::SellLimit(4, 0, 10, 15, OrderTimeInForce::AON));
    market.AddOrder(Order::BuyLimit(5, 0, 10, 5));
    market.AddOrder(Order::BuyLimit(6, 0, 10, 20, OrderTimeInForce::AON));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 3));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(45, 30));

    // Automatic matching 'All-Or-None' order
    market.AddOrder(Order::SellLimit(7, 0, 10, 15));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - 'Hidden' limit order", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10, OrderTimeInForce::GTC, 5));
    market.AddOrder(Order::BuyLimit(2, 0, 20, 20, OrderTimeInForce::GTC, 10));
    market.AddOrder(Order::BuyLimit(3, 0, 30, 30, OrderTimeInForce::GTC, 15));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(60, 0));
    REQUIRE(BookVisibleVolume(market.GetOrderBook(0)) == std::make_pair(30, 0));

    // Automatic matching with market order
    market.AddOrder(Order::SellMarket(4, 0, 55));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(5, 0));
    REQUIRE(BookVisibleVolume(market.GetOrderBook(0)) == std::make_pair(5, 0));
}

TEST_CASE("Automatic matching - stop order", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10));
    market.AddOrder(Order::BuyLimit(2, 0, 20, 20));
    market.AddOrder(Order::BuyLimit(3, 0, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(60, 0));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));

    // Automatic matching with stop order
    market.AddOrder(Order::SellStop(4, 0, 40, 60));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));

    // Add stop order
    market.AddOrder(Order::SellLimit(5, 0, 30, 30));
    market.AddOrder(Order::BuyStop(6, 0, 40, 40));
    market.AddOrder(Order::SellLimit(7, 0, 60, 60));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 2));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 90));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(1, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(40, 0));

    // Automatic matching with limit order
    market.AddOrder(Order::BuyLimit(8, 0, 40, 40));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(10, 20));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - stop order with an empty market", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    market.AddOrder(Order::SellStop(1, 0, 10, 10));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));

    market.AddOrder(Order::BuyStop(2, 0, 20, 20));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - stop-limit order", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10));
    market.AddOrder(Order::BuyLimit(2, 0, 20, 20));
    market.AddOrder(Order::BuyLimit(3, 0, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(60, 0));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));

    // Automatic matching with stop-limit orders
    market.AddOrder(Order::SellStopLimit(4, 0, 40, 20, 40));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(2, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(20, 0));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
    market.AddOrder(Order::SellStopLimit(5, 0, 30, 10, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 10));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));

    // Add stop-limit order
    market.AddOrder(Order::BuyStopLimit(6, 0, 20, 10, 10));
    market.AddOrder(Order::SellLimit(7, 0, 20, 20));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 2));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 30));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(1, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(10, 0));

    // Automatic matching with limit order
    market.AddOrder(Order::BuyLimit(7, 0, 20, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(10, 0));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - stop-limit order with an empty market", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    market.AddOrder(Order::SellStopLimit(1, 0, 10, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 30));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
    market.DeleteOrder(1);

    market.AddOrder(Order::BuyStopLimit(2, 0, 30, 10, 10));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(10, 0));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
    market.DeleteOrder(2);
}

TEST_CASE("Automatic matching - trailing stop order", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Create the market with last prices
    market.AddOrder(Order::BuyLimit(1, 0, 100, 20));
    market.AddOrder(Order::SellLimit(2, 0, 200, 20));
    market.AddOrder(Order::SellMarket(3, 0, 10));
    market.AddOrder(Order::BuyMarket(4, 0, 10));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(10, 10));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));

    // Add some trailing stop orders
    market.AddOrder(Order::TrailingBuyStop(5, 0, 1000, 10, 10, 5));
    market.AddOrder(Order::TrailingSellStopLimit(6, 0, 0, 10, 10, -1000, -500));
    REQUIRE(market.GetOrder(5)->StopPrice == 210);
    REQUIRE(market.GetOrder(6)->StopPrice == 90);
    REQUIRE(market.GetOrder(6)->Price == 100);
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(10, 10));
    REQUIRE(BookStopOrders(market.GetOrderBook(0)) == std::make_pair(1, 1));
    REQUIRE(BookStopVolume(market.GetOrderBook(0)) == std::make_pair(10, 10));
    
    // Move the market best bid price level
    market.ModifyOrder(1, 103, 20);
    REQUIRE(market.GetOrder(6)->StopPrice == 90);
    REQUIRE(market.GetOrder(6)->Price == 100);
    market.ModifyOrder(1, 120, 20);
    REQUIRE(market.GetOrder(6)->StopPrice == 108);
    REQUIRE(market.GetOrder(6)->Price == 118);

    // Move the market best ask price level. Trailing stop price will not move
    // because the last bid price = 200
    market.ModifyOrder(2, 197, 20);
    REQUIRE(market.GetOrder(5)->StopPrice == 210);
    market.ModifyOrder(2, 180, 20);
    REQUIRE(market.GetOrder(5)->StopPrice == 210);

    // Move the market best ask price level
    market.ModifyOrder(2, 197, 20);
    market.AddOrder(Order::BuyMarket(7, 0, 10));
    REQUIRE(market.GetOrder(5)->StopPrice == 210);
    market.ModifyOrder(2, 180, 20);
    market.AddOrder(Order::BuyMarket(7, 0, 10));
    REQUIRE(market.GetOrder(5)->StopPrice == 190);
}

TEST_CASE("In-Flight Mitigation", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add buy limit order
    market.AddOrder(Order::BuyLimit(1, 0, 10, 100));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(100, 0));

    // Add sell limit order
    market.AddOrder(Order::SellLimit(2, 0, 20, 100));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(100, 100));

    // Automatic matching on add limit orders
    market.AddOrder(Order::SellLimit(3, 0, 10, 20));
    market.AddOrder(Order::BuyLimit(4, 0, 20, 20));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(80, 80));

    // Mitigate orders
    market.MitigateOrder(1, 10, 150);
    market.MitigateOrder(2, 20, 50);
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(1, 1));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(130, 30));

    // Mitigate orders
    market.MitigateOrder(1, 10, 20);
    market.MitigateOrder(2, 20, 10);
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Manual matching", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    const char name[8] = "test";
    Symbol symbol = { 0, name };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Add buy limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 10));
    market.AddOrder(Order::BuyLimit(2, 0, 10, 20));
    market.AddOrder(Order::BuyLimit(3, 0, 10, 30));
    market.AddOrder(Order::BuyLimit(4, 0, 20, 10));
    market.AddOrder(Order::BuyLimit(5, 0, 20, 20));
    market.AddOrder(Order::BuyLimit(6, 0, 20, 30));
    market.AddOrder(Order::BuyLimit(7, 0, 30, 10));
    market.AddOrder(Order::BuyLimit(8, 0, 30, 20));
    market.AddOrder(Order::BuyLimit(9, 0, 30, 30));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(9, 0));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(180, 0));

    // Add sell limit orders
    market.AddOrder(Order::SellLimit(10, 0, 10, 30));
    market.AddOrder(Order::SellLimit(11, 0, 10, 20));
    market.AddOrder(Order::SellLimit(12, 0, 10, 10));
    market.AddOrder(Order::SellLimit(13, 0, 20, 30));
    market.AddOrder(Order::SellLimit(14, 0, 20, 25));
    market.AddOrder(Order::SellLimit(15, 0, 20, 10));
    market.AddOrder(Order::SellLimit(16, 0, 30, 30));
    market.AddOrder(Order::SellLimit(17, 0, 30, 20));
    market.AddOrder(Order::SellLimit(18, 0, 30, 10));
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(9, 9));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(180, 185));

    // Perform manual matching
    market.Match();
    REQUIRE(BookOrders(market.GetOrderBook(0)) == std::make_pair(3, 4));
    REQUIRE(BookVolume(market.GetOrderBook(0)) == std::make_pair(60, 65));
}
