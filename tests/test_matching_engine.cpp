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

TEST_CASE("Automatic matching - limit orders", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    Symbol symbol = { 0, "test" };
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
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 0));

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
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 9));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 180));

    // Automatic matching on add limit orders
    market.AddOrder(Order::SellLimit(19, 0, 30, 5));
    market.AddOrder(Order::SellLimit(20, 0, 30, 25));
    market.AddOrder(Order::SellLimit(21, 0, 30, 15));
    market.AddOrder(Order::SellLimit(22, 0, 30, 20));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(6, 10));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(120, 185));

    // Automatic matching on several levels
    market.AddOrder(Order::BuyLimit(23, 0, 60, 105));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(6, 5));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(120, 80));

    // Automatic matching on modify order
    market.ModifyOrder(15, 20, 20);
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(5, 4));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(100, 70));

    // Automatic matching on replace order
    market.ReplaceOrder(2, 24, 70, 100);
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(5, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(110, 0));
    market.ReplaceOrder(1, Order::SellLimit(25, 0, 0, 100));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - market orders", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    Symbol symbol = { 0, "test" };
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
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 0));

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
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 9));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 180));

    // Automatic matching on add market order
    market.AddOrder(Order::SellMarket(19, 0, 15));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(8, 9));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(165, 180));

    // Automatic matching on add market order with slippage
    market.AddOrder(Order::SellMarket(20, 0, 100, 0));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(6, 9));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(120, 180));
    market.AddOrder(Order::BuyMarket(21, 0, 160, 20));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(6, 2));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(120, 20));

    // Automatic matching on add market order with reaching end of the book
    market.AddOrder(Order::SellMarket(22, 0, 1000));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(0, 2));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(0, 20));
    market.AddOrder(Order::BuyMarket(23, 0, 1000));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(0, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(0, 0));
}

TEST_CASE("Automatic matching - limit 'All-Or-None' orders", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    Symbol symbol = { 0, "test" };
    market.AddSymbol(symbol);
    market.AddOrderBook(symbol);

    // Enable automatic matching
    market.EnableMatching();

    // Add limit orders
    market.AddOrder(Order::BuyLimit(1, 0, 10, 20, true));
    market.AddOrder(Order::SellLimit(2, 0, 10, 10, true));
    market.AddOrder(Order::SellLimit(3, 0, 10, 5, false));
    market.AddOrder(Order::SellLimit(4, 0, 10, 15, true));
    market.AddOrder(Order::BuyLimit(5, 0, 10, 5, false));
    market.AddOrder(Order::BuyLimit(6, 0, 10, 15, true));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(3, 3));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(40, 30));

    // Automatic matching 'All-Or-None' orders
    market.AddOrder(Order::SellLimit(7, 0, 10, 15, false));
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(0, 1));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(0, 5));
}

TEST_CASE("Manual matching", "[CppTrader][Matching]")
{
    MarketManager market;

    // Prepare symbol & order book
    Symbol symbol = { 0, "test" };
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
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 0));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 0));

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
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(9, 9));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(180, 185));

    // Perform manual matching
    market.Match();
    REQUIRE(BookOrders(*market.GetOrderBook(0)) == std::make_pair(3, 4));
    REQUIRE(BookVolume(*market.GetOrderBook(0)) == std::make_pair(60, 65));
}
