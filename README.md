# CppTrader

[![Linux (clang)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-clang.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-clang.yml)
[![Linux (gcc)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-gcc.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-gcc.yml)
<br/>
[![MacOS](https://github.com/chronoxor/CppTrader/actions/workflows/build-macos.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-macos.yml)
<br/>
[![Windows (Cygwin)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-cygwin.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-cygwin.yml)
[![Windows (MSYS2)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-msys2.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-msys2.yml)
[![Windows (MinGW)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-mingw.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-mingw.yml)
[![Windows (Visual Studio)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-vs.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-vs.yml)

C++ Trader is a set of components for building high performance Trading Platform:
* Ultra fast matching engine
* Order book processor
* NASDAQ ITCH handler

[CppTrader API reference](https://chronoxor.github.io/CppTrader/index.html)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
  * [Performance](#performance)
    * [NASDAQ ITCH handler](#nasdaq-itch-handler)
    * [Market manager](#market-manager)
    * [Market manager (optimized version)](#market-manager-optimized-version)
    * [Market manager (aggressive optimized version)](#market-manager-aggressive-optimized-version)

# Features
* Cross platform (Linux, MacOS, Windows)
* Benchmarks
* Examples
* Tests
* [Doxygen](http://www.doxygen.org) API documentation
* Continuous integration ([Travis CI](https://travis-ci.com), [AppVeyor](https://www.appveyor.com))

# Requirements
* Linux
* MacOS
* Windows
* [cmake](https://www.cmake.org)
* [gcc](https://gcc.gnu.org)
* [git](https://git-scm.com)
* [gil](https://github.com/chronoxor/gil.git)
* [python3](https://www.python.org)

Optional:
* [clang](https://clang.llvm.org)
* [CLion](https://www.jetbrains.com/clion)
* [Cygwin](https://cygwin.com)
* [MSYS2](https://www.msys2.org)
* [MinGW](https://mingw-w64.org/doku.php)
* [Visual Studio](https://www.visualstudio.com)

# How to build?

### Linux: install required packages
```shell
sudo apt-get install -y binutils-dev uuid-dev
```

### Install [gil (git links) tool](https://github.com/chronoxor/gil)
```shell
pip3 install gil
```

### Setup repository
```shell
git clone https://github.com/chronoxor/CppTrader.git
cd CppTrader
gil update
```

### Linux
```shell
cd build
./unix.sh
```

### MacOS
```shell
cd build
./unix.sh
```

### Windows (Cygwin)
```shell
cd build
unix.bat
```

### Windows (MSYS2)
```shell
cd build
unix.bat
```

### Windows (MinGW)
```shell
cd build
mingw.bat
```

### Windows (Visual Studio)
```shell
cd build
vs.bat
```

# Performance

Here comes several micro-benchmarks for trading components.

Benchmark environment is the following:
```
CPU architecutre: Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz
CPU logical cores: 8
CPU physical cores: 4
CPU clock speed: 3.998 GHz
CPU Hyper-Threading: enabled
RAM total: 31.962 GiB
RAM free: 21.623 GiB

OS version: Microsoft Windows 8 Enterprise Edition (build 9200), 64-bit
OS bits: 64-bit
Process bits: 64-bit
Process configuaraion: release
```

## NASDAQ ITCH handler

Benchmark measures the performance of the [NASDAQ ITCH handler](https://github.com/chronoxor/CppTrader/blob/master/include/trader/providers/nasdaq/itch_handler.h).
It shows how fast it can parse and handle ITCH messages from the input stream.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-itch_handler](https://github.com/chronoxor/CppTrader/blob/master/performance/itch_handler.cpp) < 01302017.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 6.831 s
Total ITCH messages: 283238832
ITCH message latency: 24 ns
ITCH message throughput: 41460256 msg/s
```

## Market manager

Benchmark measures the performance of the [Market manager](https://github.com/chronoxor/CppTrader/blob/master/include/trader/matching/market_manager.h ).
It shows how fast it can handle orders operations (add, reduce, modify, delete,
execute) and build an order book.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-market_manager](https://github.com/chronoxor/CppTrader/blob/master/performance/market_manager.cpp) < 01302017.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 1:27.616 m
Total ITCH messages: 283238832
ITCH message latency: 309 ns
ITCH message throughput: 3232727 msg/s
Total market updates: 631217516
Market update latency: 138 ns
Market update throughput: 7204359 upd/s

Market statistics:
Max symbols: 8371
Max order books: 8371
Max order book levels: 2422
Max order book orders: 2975
Max orders: 1647972

Order statistics:
Add order operations: 152865456
Update order operations: 7037619
Delete order operations: 152865456
Execute order operations: 5663712
```

## Market manager (optimized version)

This is an optimized version of the Market manager. Optimization tricks are the
following:

* Symbols and order books are stored in fixed size pre-allocated arrays.
* Orders are stored in the pre-allocated array instead of HashMap. This gives
O(1) for all orders operations with no overhead (get, insert, update, delete).
* Orders linked list is not maintained for price levels, just orders count.
* Price levels are stored in sorted arrays instead of Red-Black trees. The sort
order keeps best prices (best bid / best ask) at the end of arrays which gives
good CPU cache locality and near to O(1) search time for orders with close to
market prices, but has a penalty for orders with far from market prices!
* Price levels are taken from the pool, which is implemented using a
pre-allocated array with O(1) for create and delete each price level.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-market_manager_optimized](https://github.com/chronoxor/CppTrader/blob/master/performance/market_manager_optimized.cpp) < 01302017.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 34.150 s
Total ITCH messages: 283238832
ITCH message latency: 120 ns
ITCH message throughput: 8293747 msg/s
Total market updates: 631217516
Market update latency: 54 ns
Market update throughput: 18483195 upd/s

Market statistics:
Max symbols: 8371
Max order books: 8371
Max order book levels: 38
Max orders: 1647972

Order statistics:
Add order operations: 152865456
Update order operations: 7037619
Delete order operations: 152865456
Execute order operations: 5663712
```

## Market manager (aggressive optimized version)

This is a very aggressive optimized version of the Market manager. It shows
values of latency and throughput close to optimal with the cost of some more
optimization tricks which might be hard to keep in real trading platforms:

* Symbols are not maintained
* Orders and price limits structures are optimized to be optimal. Most of useful
filds are removed.
* Price values are stored as signed 32-bit integer values. Positive values for
bids and negative values for asks.
* Market handler is not used. No way to receive notifications from the Market
manager.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-market_manager_optimized_aggressive](https://github.com/chronoxor/CppTrader/blob/master/performance/market_manager_optimized_aggressive.cpp) < 01302017.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0
Processing time: 29.047 s
Total ITCH messages: 283238832
ITCH messages latency: 102 ns
ITCH messages throughput: 9751044 msg/s
```
