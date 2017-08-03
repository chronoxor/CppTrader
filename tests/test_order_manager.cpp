//
// Created by Ivan Shynkarenka on 31.07.2017
//

#include "catch.hpp"

#include "trader/order_manager.h"

using namespace CppTrader;

TEST_CASE("Order manager", "[CppTrader]")
{
    OrderManager orders;
    REQUIRE(orders.empty());

    REQUIRE(orders.size() == 0);
    orders.AddOrder(Order(1, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0));
    REQUIRE(orders.size() == 1);
    orders.AddOrder(Order(2, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0));
    REQUIRE(orders.size() == 2);
    orders.AddOrder(Order(3, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0));
    REQUIRE(orders.size() == 3);

    REQUIRE(orders.GetOrder(1) != nullptr);
    REQUIRE(orders.GetOrder(2) != nullptr);
    REQUIRE(orders.GetOrder(3) != nullptr);
    REQUIRE(orders.GetOrder(4) == nullptr);

    orders.DeleteOrder(1);
    REQUIRE(orders.size() == 2);
    orders.DeleteOrder(2);
    REQUIRE(orders.size() == 1);
    orders.DeleteOrder(3);
    REQUIRE(orders.size() == 0);

    REQUIRE(orders.empty());
}
