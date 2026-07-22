# CppTrader

[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Release](https://img.shields.io/github/release/chronoxor/CppTrader.svg?sort=semver)](https://github.com/chronoxor/CppTrader/releases)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
<br/>
[![Linux (clang)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-clang.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-clang.yml)
[![Linux (gcc)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-gcc.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-linux-gcc.yml)
[![MacOS](https://github.com/chronoxor/CppTrader/actions/workflows/build-macos.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-macos.yml)
<br/>
[![Windows (Cygwin)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-cygwin.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-cygwin.yml)
[![Windows (MSYS2)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-msys2.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-msys2.yml)
[![Windows (MinGW)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-mingw.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-mingw.yml)
[![Windows (Visual Studio)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-vs.yml/badge.svg)](https://github.com/chronoxor/CppTrader/actions/workflows/build-windows-vs.yml)

C++ Trader is a set of components written in **C++23** for building high performance Trading Platform:
* Ultra fast matching engine
* Order book processor
* NASDAQ ITCH handler

[CppTrader API reference](https://chronoxor.github.io/CppTrader/index.html)

# Contents
  * [Features](#features)
  * [Requirements](#requirements)
  * [How to build?](#how-to-build)
  * [Performance](#performance)
    * [NASDAQ ITCH handler performance benchmark](#nasdaq-itch-handler-performance-benchmark)
    * [Matching engine performance benchmark](#matching-engine-performance-benchmark)
  * [Contributing](#contributing)
  * [Contributors](#contributors)
  * [Sponsors](#sponsors)
  * [License](#license)

# Features
* Matching engine
  * Market, limit, stop, stop limit orders
  * IOC (Immediate-or-Cancel) orders
  * FOK (Fill-or-Kill) orders
  * GTD (Good-till-Date) orders
  * PEG (Peg) orders
  * Iceberg orders
  * Order book processor
* NASDAQ ITCH handler
  * ITCH 5.0 market data protocol processing
* Components
  * High Resolution Time
  * High Performance Timers
  * High Performance Random
  * High Performance Hashing
  * High Performance Cache
  * Memory Pool Allocator

# Requirements
* Linux
* MacOS
* Windows
* **C++23 Standard Compiler**
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
```
sudo apt-get update
sudo apt-get install build-essential cmake git python3
```

### Linux: build CppTrader project
```
git clone https://github.com/chronoxor/CppTrader.git
cd CppTrader
gil update
cmake -DCMAKE_INSTALL_PREFIX=../tmp .
cmake --build .
cmake --install .
```

### OSX: install required packages
```
brew install cmake git python3
```

### OSX: build CppTrader project
```
git clone https://github.com/chronoxor/CppTrader.git
cd CppTrader
gil update
cmake -DCMAKE_INSTALL_PREFIX=../tmp .
cmake --build .
cmake --install .
```

### Windows (Cygwin): install required packages
```
cygwin setup
packages: cmake, gcc-core, gcc-g++, git, make, python3
```

### Windows (Cygwin): build CppTrader project
```
git clone https://github.com/chronoxor/CppTrader.git
cd CppTrader
gil update
cmake -DCMAKE_INSTALL_PREFIX=../tmp -G "Unix Makefiles" .
cmake --build .
cmake --install .
```

### Windows (MSYS2): install required packages
```
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake git python3
```

### Windows (MSYS2): build CppTrader project
```
git clone https://github.com/chronoxor/CppTrader.git
cd CppTrader
gil update
cmake -DCMAKE_INSTALL_PREFIX=../tmp -G "MSYS Makefiles" .
cmake --build .
cmake --install .
```

### Windows (MinGW-w64): install required packages
```
Download and install MinGW-w64 with GCC from https://mingw-w64.org
Download and install CMake from https://www.cmake.org
Download and install Git from https://git-scm.com
Download and install Python from https://www.python.org
```

### Windows (MinGW-w64): build CppTrader project
```
git clone https://github.com/chronoxor/CppTrader.git
cd CppTrader
gil update
cmake -DCMAKE_INSTALL_PREFIX=../tmp -G "MinGW Makefiles" .
cmake --build .
cmake --install .
```

### Windows (Visual Studio): install required packages
```
Download and install Visual Studio 2022 from https://visualstudio.com
Download and install CMake from https://www.cmake.org
Download and install Git from https://git-scm.com
Download and install Python from https://www.python.org
```

### Windows (Visual Studio): build CppTrader project
Open "Developer Command Prompt for VS 2022"
```
git clone https://github.com/chronoxor/CppTrader.git
cd CppTrader
gil update
cmake -DCMAKE_INSTALL_PREFIX=../tmp -G "Visual Studio 17 2022" .
cmake --build .
cmake --install .
```

# Performance

### NASDAQ ITCH handler performance benchmark

|                  Method                  |     Time     |
|-----------------------------------------|-------------:|
| NASDAQ ITCH handler - 12800000 messages |  66.272 ms   |
| NASDAQ ITCH handler - 12800000 messages |  66.272 ms   |

### Matching engine performance benchmark

|                  Method                  |     Time     |
|-----------------------------------------|-------------:|
| Matching engine - 2000000 limit orders   |  44.897 ms   |
| Matching engine - 2000000 market orders  |  82.221 ms   |
| Matching engine - 1000000 IOC orders     |  36.809 ms   |

# Contributing

See the [CONTRIBUTING](CONTRIBUTING.md) file for details.

# Contributors

* [Ivan Shynkarenka](https://github.com/chronoxor)

# Sponsors

* [CyberInfrastructure](https://cyberinfrastructure.com)
* [Cybergos](https://cybergos.com)

# License

CppTrader is available under the [MIT license](LICENSE).