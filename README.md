# CppTrader

[![Linux (clang)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-clang.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-clang.yml)
[![Linux (gcc)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-gcc.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-gcc.yml)
<br/>
[![MacOS](https://github.com/chronoxor/CppTrader/actions/workflows/build-macos.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-macos.yml)
<br/>
[![Windows (Cygwin)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-cygwin.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-cygwin.yml)
[![Windows (MinGW)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-mingw.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-mingw.yml)
[![Windows (MSYS2)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-msys2.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-msys2.yml)
[![Windows (Visual Studio)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-vs.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-vs.yml)

C++ Trader is a set of components for building high performance Trading Platform:
* Ultra fast matching engine
* Order book processor
* NASDAQ ITCH handler

[CppTrader API reference](https://chronoxor.github.io/CppTrader/index.html)

# Diploma Thesis
## Performance Evaluation and Improvement of System for Execution of Financial Transactions

As part of my diploma thesis, on CppTraders Matching Engine I had to:
* Understand how the system works, its architecture, its data structures and algorithms, as well as familiarize with financial terms as needed
* Perform workload analysis
* Use gperftools (profiling tool) in order to find bottlenecks in the systems data structures and algorithms
* Replace the problematic data structures with better performing ones
* Evaluate the performance of the system using various measurements and metrics (e.g. throughput)

[Click here for Thesis Paper and Presentation](https://github.com/kasselouris/CppTrader/tree/master/thesis_docs)

To show some of my work I also made [cpptrader-performance-matching_engine_optimized](https://github.com/kasselouris/CppTrader/blob/master/performance/matching_engine_optimized.cpp) (more on this [here](#matching-engine-optimized-version---part-of-my-diploma-thesis)) based on code from [Ivan Shynkarenka (chronoxor)](https://github.com/chronoxor) who is the creator of this amazing open-source project.

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
  * [Performance](#performance)
    * [NASDAQ ITCH Handler](#nasdaq-itch-handler)
    * [Market Manager](#market-manager)
    * [Market Manager (optimized version)](#market-manager-optimized-version)
    * [Market Manager (aggressive optimized version)](#market-manager-aggressive-optimized-version)
    * [Matching Engine](#matching-engine)
    * [Matching Engine (optimized version) - Part of my Diploma Thesis](#matching-engine-optimized-version---part-of-my-diploma-thesis)

# Features
* Cross platform (Linux, MacOS, Windows)
* Benchmarks
* Examples
* Tests
* [Doxygen](http://www.doxygen.org) API documentation
* Continuous integration ([Travis CI](https://travis-ci.com), [AppVeyor](https://www.appveyor.com))

# Requirements
* Linux (binutils-dev uuid-dev)
* MacOS
* Windows 10
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
CPU architecutre: Intel(R) Core(TM) i5-6400 CPU @ 2.7GHz
CPU logical cores: 3
CPU physical cores: 3
RAM total: 11.8 GB
RAM free: 9.8 GB
Swap memory: 1.4 GB
Machine: Vmware 15.6
OS: Linux
OS bits: 64-bit
Distro: Ubuntu 20.04.2 LTS (Focal Fossa)
Process configuaraion: Release
Cache: warmed
```

## NASDAQ ITCH Handler

Benchmark measures the performance of the [NASDAQ ITCH handler](https://github.com/kasselouris/CppTrader/blob/master/include/trader/providers/nasdaq/itch_handler.h).
It shows how fast it can parse and handle ITCH messages from the input stream.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-itch_handler](https://github.com/kasselouris/CppTrader/blob/master/performance/itch_handler.cpp) < 12302019.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 31.512 s
Total ITCH messages: 268744780
ITCH message latency: 117 ns
ITCH message throughput: 8528274 msg/s
```

## Market Manager

Benchmark measures the performance of the [Market manager](https://github.com/kasselouris/CppTrader/blob/master/include/trader/matching/market_manager.h).
It shows how fast it can handle orders operations (add, reduce, modify, delete,
execute) and build an order book.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-market_manager](https://github.com/kasselouris/CppTrader/blob/master/performance/market_manager.cpp) < 12302019.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 3:07.481 m
Total ITCH messages: 268744780
ITCH message latency: 697 ns
ITCH message throughput: 1433446 msg/s
Total market updates: 575602561
Market update latency: 325 ns
Market update throughput: 3070183 upd/s

Market statistics: 
Max symbols: 8906
Max order books: 8906
Max order book levels: 4227
Max order book orders: 3415
Max orders: 1924078

Order statistics: 
Add order operations: 140270523
Update order operations: 4339958
Delete order operations: 140270523
Execute order operations: 5822741
```

## Market Manager (optimized version)

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

* [cpptrader-performance-market_manager_optimized](https://github.com/kasselouris/CppTrader/blob/master/performance/market_manager_optimized.cpp) < 12302019.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 55.749 s
Total ITCH messages: 268744780
ITCH message latency: 207 ns
ITCH message throughput: 4820587 msg/s
Total market updates: 575602561
Market update latency: 96 ns
Market update throughput: 10324823 upd/s

Market statistics: 
Max symbols: 8906
Max order books: 8906
Max order book levels: 37
Max orders: 1924078

Order statistics: 
Add order operations: 140270523
Update order operations: 4339958
Delete order operations: 140270523
Execute order operations: 5822741
```

## Market Manager (aggressive optimized version)

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

* [cpptrader-performance-market_manager_optimized_aggressive](https://github.com/kasselouris/CppTrader/blob/master/performance/market_manager_optimized_aggressive.cpp) < 12302019.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 56.722 s
Total ITCH messages: 268744780
ITCH message latency: 211 ns
ITCH message throughput: 4737884 msg/s
```
> In my system market manager aggressive optimized versions performance is almost the same as the optimized versions one. In higher end systems it performs slightly better.

## Matching Engine
It handles add, delete, reduce, replace order operations. It also builds the order book the same way as market manager version, though now it does not handle execute order operations and we are responsible for the matching of the orders. 

> There is a slight deviation in the execution of orders between market manager versions and matching engine ones.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-matching_engine](https://github.com/kasselouris/CppTrader/blob/master/performance/matching_engine.cpp) < 12302019.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 3:06.655 m

Load messages:
Total ITCH messages: 268744780
Total actual used ITCH messages (Real messages): 257428102
---------------
Symbol messages: 8906
Add order messages: 118631456
Reduce order messages: 2787676
Delete order messages: 114360997
Replace order messages: 21639067

Performance Statistics:
ITCH message latency: 694 ns
ITCH message throughput: 1439786 msg/s
Total market updates: 574983404
Market update latency: 324 ns
Market update throughput: 3080445 upd/s

Market statistics: 
Max symbols: 8906
Max order books: 8906
Max orders: 2089674
Max AVL-tree levels: 4272
Max level orders: 3415

Order statistics: 
Add order operations: 139815986
Update order operations: 5084981
Delete order operations: 139630304
Execute order operations: 11828382
```

## Matching Engine (optimized version) - Part of my Diploma Thesis
This is an optimized version of the Matching Engine. Optimization tricks are the
following:

* Orders are stored in the pre-allocated array instead of HashMap. This gives
O(1) for all orders operations with no overhead (get, insert, update, delete).
* Price levels are stored in sorted arrays instead of AVL trees. The sort
order keeps best prices (best bid / best ask) at the end of arrays which gives
good CPU cache locality and near to O(1) search time for orders with close to
market prices, but has a penalty for orders with far from market prices!
* Price level and order objects are taken from their respective pool, which is implemented using a
pre-allocated array with O(1) for creating and deleting each object.
* Symbols and order books are stored in fixed size pre-allocated arrays.
* Increased x4 input buffer size from 8192 to 32768 and disabled C and C++ input/output buffer synchronization for increased I/O performance.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-matching_engine_optimized](https://github.com/kasselouris/CppTrader/blob/master/performance/matching_engine_optimized.cpp) < 12302019.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 1:29.600 m

Load messages:
Total ITCH messages: 268744780
Total actual used ITCH messages (Real messages): 257428102
---------------
Symbol messages: 8906
Add order messages: 118631456
Reduce order messages: 2787676
Delete order messages: 114360997
Replace order messages: 21639067

Performance Statistics:
ITCH message latency: 333 ns
ITCH message throughput: 2999376 msg/s
Total market updates: 574983404
Market update latency: 155 ns
Market update throughput: 6417210 upd/s

Market statistics: 
Max symbols: 8906
Max order books: 8906
Max orders: 2089674
Max vector levels: 4272
Max level orders: 3415

Order statistics: 
Add order operations: 139815986
Update order operations: 5084981
Delete order operations: 139630304
Execute order operations: 11828382
```
