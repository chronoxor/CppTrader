//
// Created by Ivan Shynkarenka on 31.07.2017
//

#include "catch.hpp"

#include "trader/symbol_manager.h"

using namespace CppTrader;

TEST_CASE("Symbol manager", "[CppTrader]")
{
    SymbolManager symbols;
    REQUIRE(symbols.empty());

    REQUIRE(symbols.size() == 0);
    REQUIRE(symbols.AddSymbol(Symbol(1, "Symbol1")));
    REQUIRE(symbols.size() == 1);
    REQUIRE(symbols.AddSymbol(Symbol(2, "Symbol2")));
    REQUIRE(symbols.size() == 2);
    REQUIRE(symbols.AddSymbol(Symbol(3, "Symbol3")));
    REQUIRE(symbols.size() == 3);

    REQUIRE(!symbols.AddSymbol(Symbol(1, "Symbol")));
    REQUIRE(!symbols.AddSymbol(Symbol(2, "Symbol")));
    REQUIRE(!symbols.AddSymbol(Symbol(3, "Symbol")));

    REQUIRE(symbols.GetSymbol(0) == nullptr);
    REQUIRE(symbols.GetSymbol(1) != nullptr);
    REQUIRE(symbols.GetSymbol(2) != nullptr);
    REQUIRE(symbols.GetSymbol(3) != nullptr);
    REQUIRE(symbols.GetSymbol(4) == nullptr);

    REQUIRE(symbols.GetSymbolByName("Symbol") == nullptr);
    REQUIRE(symbols.GetSymbolByName("Symbol1") != nullptr);
    REQUIRE(symbols.GetSymbolByName("Symbol2") != nullptr);
    REQUIRE(symbols.GetSymbolByName("Symbol3") != nullptr);

    REQUIRE(!symbols.DeleteSymbol(0));
    REQUIRE(symbols.size() == 3);
    REQUIRE(symbols.DeleteSymbol(1));
    REQUIRE(symbols.size() == 2);
    REQUIRE(symbols.DeleteSymbol(2));
    REQUIRE(symbols.size() == 1);
    REQUIRE(symbols.DeleteSymbol(3));
    REQUIRE(symbols.size() == 0);
    REQUIRE(!symbols.DeleteSymbol(0));

    REQUIRE(symbols.empty());
}
