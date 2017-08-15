//
// Created by Ivan Shynkarenka on 15.08.2017
//

#include "catch.hpp"

#include "trader/matching/market_manager.h"

using namespace CppCommon;
using namespace CppTrader::Matching;

namespace {

std::pair<int, int> BookOrders(const OrderBook& order_book)
{
    int bid_orders = 0;
    for (auto& bid : order_book.bids())
        bid_orders += (int)bid.Orders;

    int ask_orders = 0;
    for (auto& ask : order_book.asks())
        ask_orders += (int)ask.Orders;

    return std::make_pair(bid_orders, ask_orders);
}

std::pair<int, int> BookVolume(const OrderBook& order_book)
{
    int bid_volume = 0;
    for (auto& bid : order_book.bids())
        bid_volume += (int)bid.Volume;

    int ask_volume = 0;
    for (auto& ask : order_book.asks())
        ask_volume += (int)ask.Volume;

    return std::make_pair(bid_volume, ask_volume);
}

}

TEST_CASE("Automatic matching", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    Symbol symbol = { 0, "test" };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add buy orders
    market.AddOrder(Order{ 1, 0, OrderType::LIMIT, OrderSide::BUY, 10, 10 });
    market.AddOrder(Order{ 2, 0, OrderType::LIMIT, OrderSide::BUY, 10, 20 });
    market.AddOrder(Order{ 3, 0, OrderType::LIMIT, OrderSide::BUY, 10, 30 });
    market.AddOrder(Order{ 4, 0, OrderType::LIMIT, OrderSide::BUY, 20, 10 });
    market.AddOrder(Order{ 5, 0, OrderType::LIMIT, OrderSide::BUY, 20, 20 });
    market.AddOrder(Order{ 6, 0, OrderType::LIMIT, OrderSide::BUY, 20, 30 });
    market.AddOrder(Order{ 7, 0, OrderType::LIMIT, OrderSide::BUY, 30, 10 });
    market.AddOrder(Order{ 8, 0, OrderType::LIMIT, OrderSide::BUY, 30, 20 });
    market.AddOrder(Order{ 9, 0, OrderType::LIMIT, OrderSide::BUY, 30, 30 });
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 0));

    // Add sell orders
    market.AddOrder(Order{ 10, 0, OrderType::LIMIT, OrderSide::SELL, 40, 30 });
    market.AddOrder(Order{ 11, 0, OrderType::LIMIT, OrderSide::SELL, 40, 20 });
    market.AddOrder(Order{ 12, 0, OrderType::LIMIT, OrderSide::SELL, 40, 10 });
    market.AddOrder(Order{ 13, 0, OrderType::LIMIT, OrderSide::SELL, 50, 30 });
    market.AddOrder(Order{ 14, 0, OrderType::LIMIT, OrderSide::SELL, 50, 20 });
    market.AddOrder(Order{ 15, 0, OrderType::LIMIT, OrderSide::SELL, 50, 10 });
    market.AddOrder(Order{ 16, 0, OrderType::LIMIT, OrderSide::SELL, 60, 30 });
    market.AddOrder(Order{ 17, 0, OrderType::LIMIT, OrderSide::SELL, 60, 20 });
    market.AddOrder(Order{ 18, 0, OrderType::LIMIT, OrderSide::SELL, 60, 10 });
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 9));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 180));

    // Automatic matching on orders add
    market.AddOrder(Order{ 19, 0, OrderType::LIMIT, OrderSide::SELL, 30, 5 });
    market.AddOrder(Order{ 20, 0, OrderType::LIMIT, OrderSide::SELL, 30, 25 });
    market.AddOrder(Order{ 21, 0, OrderType::LIMIT, OrderSide::SELL, 30, 15 });
    market.AddOrder(Order{ 22, 0, OrderType::LIMIT, OrderSide::SELL, 30, 20 });
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(6, 10));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(120, 185));

    // Automatic matching on several levels
    market.AddOrder(Order{ 23, 0, OrderType::LIMIT, OrderSide::BUY, 60, 105 });
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(6, 5));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(120, 80));

    // Automatic matching on order modify
    market.ModifyOrder(15, 20, 20);
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(5, 4));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(100, 70));

    // Automatic matching on order replace
    market.ReplaceOrder(2, 24, 70, 100);
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(5, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(110, 0));
    market.ReplaceOrder(1, Order{ 25, 0, OrderType::LIMIT, OrderSide::SELL, 0, 100 });
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Manual matching", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    Symbol symbol = { 0, "test" };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Add buy orders
    market.AddOrder(Order{ 1, 0, OrderType::LIMIT, OrderSide::BUY, 10, 10 });
    market.AddOrder(Order{ 2, 0, OrderType::LIMIT, OrderSide::BUY, 10, 20 });
    market.AddOrder(Order{ 3, 0, OrderType::LIMIT, OrderSide::BUY, 10, 30 });
    market.AddOrder(Order{ 4, 0, OrderType::LIMIT, OrderSide::BUY, 20, 10 });
    market.AddOrder(Order{ 5, 0, OrderType::LIMIT, OrderSide::BUY, 20, 20 });
    market.AddOrder(Order{ 6, 0, OrderType::LIMIT, OrderSide::BUY, 20, 30 });
    market.AddOrder(Order{ 7, 0, OrderType::LIMIT, OrderSide::BUY, 30, 10 });
    market.AddOrder(Order{ 8, 0, OrderType::LIMIT, OrderSide::BUY, 30, 20 });
    market.AddOrder(Order{ 9, 0, OrderType::LIMIT, OrderSide::BUY, 30, 30 });
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 0));

    // Add sell orders
    market.AddOrder(Order{ 10, 0, OrderType::LIMIT, OrderSide::SELL, 10, 30 });
    market.AddOrder(Order{ 11, 0, OrderType::LIMIT, OrderSide::SELL, 10, 20 });
    market.AddOrder(Order{ 12, 0, OrderType::LIMIT, OrderSide::SELL, 10, 10 });
    market.AddOrder(Order{ 13, 0, OrderType::LIMIT, OrderSide::SELL, 20, 30 });
    market.AddOrder(Order{ 14, 0, OrderType::LIMIT, OrderSide::SELL, 20, 25 });
    market.AddOrder(Order{ 15, 0, OrderType::LIMIT, OrderSide::SELL, 20, 10 });
    market.AddOrder(Order{ 16, 0, OrderType::LIMIT, OrderSide::SELL, 30, 30 });
    market.AddOrder(Order{ 17, 0, OrderType::LIMIT, OrderSide::SELL, 30, 20 });
    market.AddOrder(Order{ 18, 0, OrderType::LIMIT, OrderSide::SELL, 30, 10 });
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 9));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 185));

    // Perform manual matching
    market.Match();
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(3, 4));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(60, 65));
}
