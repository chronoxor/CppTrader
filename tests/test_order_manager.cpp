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
    REQUIRE(orders.RegisterOrder(Order(1, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0)));
    REQUIRE(orders.size() == 1);
    REQUIRE(orders.RegisterOrder(Order(2, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0)));
    REQUIRE(orders.size() == 2);
    REQUIRE(orders.RegisterOrder(Order(3, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0)));
    REQUIRE(orders.size() == 3);

    REQUIRE(!orders.RegisterOrder(Order(1, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0)));
    REQUIRE(!orders.RegisterOrder(Order(2, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0)));
    REQUIRE(!orders.RegisterOrder(Order(3, 0, OrderType::LIMIT, OrderSide::BUY, 0, 0)));

    REQUIRE(orders.GetOrder(0) == nullptr);
    REQUIRE(orders.GetOrder(1) != nullptr);
    REQUIRE(orders.GetOrder(2) != nullptr);
    REQUIRE(orders.GetOrder(3) != nullptr);
    REQUIRE(orders.GetOrder(4) == nullptr);

    REQUIRE(!orders.UnregisterOrder(0));
    REQUIRE(orders.size() == 3);
    REQUIRE(orders.UnregisterOrder(1));
    REQUIRE(orders.size() == 2);
    REQUIRE(orders.UnregisterOrder(2));
    REQUIRE(orders.size() == 1);
    REQUIRE(orders.UnregisterOrder(3));
    REQUIRE(orders.size() == 0);
    REQUIRE(!orders.UnregisterOrder(0));

    REQUIRE(orders.empty());
}
