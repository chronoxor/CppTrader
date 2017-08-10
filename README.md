# CppTrader

[![Linux build status](https://img.shields.io/travis/chronoxor/CppTrader/master.svg?label=Linux)](https://travis-ci.org/chronoxor/CppTrader)
[![OSX build status](https://img.shields.io/travis/chronoxor/CppTrader/master.svg?label=OSX)](https://travis-ci.org/chronoxor/CppTrader)
[![Cygwin build status](https://img.shields.io/appveyor/ci/chronoxor/CppTrader/master.svg?label=Cygwin)](https://ci.appveyor.com/project/chronoxor/CppTrader)
[![MinGW build status](https://img.shields.io/appveyor/ci/chronoxor/CppTrader/master.svg?label=MinGW)](https://ci.appveyor.com/project/chronoxor/CppTrader)
[![Windows build status](https://img.shields.io/appveyor/ci/chronoxor/CppTrader/master.svg?label=Windows)](https://ci.appveyor.com/project/chronoxor/CppTrader)

C++ Trader is a set of components for building a Trading Platform.

[CppTrader API reference](http://chronoxor.github.io/CppTrader/index.html)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
    * [Clone repository with submodules](#clone-repository-with-submodules)
    * [Linux](#linux)
    * [OSX](#osx)
    * [Windows (Cygwin)](#windows-cygwin)
    * [Windows (MinGW)](#windows-mingw)
    * [Windows (MinGW with MSYS)](#windows-mingw-with-msys)
    * [Windows (Visual Studio)](#windows-visual-studio)
  * [Performance](#performance)
    * [NASDAQ ITCH handler](#nasdaq-itch-handler)
    * [Market manager](#market-manager)

# Features
* Cross platform (Linux, OSX, Windows)
* Benchmarks
* Examples
* Tests
* [Doxygen](http://www.doxygen.org) API documentation
* Continuous integration ([Travis CI](https://travis-ci.com), [AppVeyor](https://www.appveyor.com))

# Requirements
* Linux
* OSX
* Windows 7 / Windows 10
* [CMake](http://www.cmake.org)
* [GIT](https://git-scm.com)
* [GCC](https://gcc.gnu.org)

Optional:
* [Clang](http://clang.llvm.org)
* [Clion](https://www.jetbrains.com/clion)
* [MinGW](http://mingw-w64.org/doku.php)
* [Visual Studio](https://www.visualstudio.com)

# How to build?

## Clone repository with submodules
```
git clone https://github.com/chronoxor/CppTrader.git CppTrader
cd CppTrader
git submodule update --init --recursive --remote
```

## Linux
```
cd build
./unix.sh
```

## OSX
```
cd build
./unix.sh
```

## Windows (Cygwin)
```
cd build
cygwin.bat
```

## Windows (MinGW)
```
cd build
mingw.bat
```

## Windows (Visual Studio)
```
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
ITCH message throughput: 41460256 messages per second
```

## Market manager

Benchmark measures the performance of the [Market manager](https://github.com/chronoxor/CppTrader/blob/master/include/trader/market_manager.h).
It shows how fast it can handle orders operations (add, reduce, modify, delete,
execute) and build an order book.

Sample ITCH file could be downloaded from ftp://emi.nasdaq.com/ITCH

* [cpptrader-performance-market_manager](https://github.com/chronoxor/CppTrader/blob/master/performance/market_manager.cpp) < 01302017.NASDAQ_ITCH50
```
ITCH processing...Done!

Errors: 0

Processing time: 1:37.421 m
Total ITCH messages: 283238832
ITCH message latency: 343 ns
ITCH message throughput: 2907346 messages per second
Total market updates: 631217516
Market update latency: 154 ns
Market update throughput: 6479223 updates per second

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
