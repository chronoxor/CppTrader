# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository setup

The `build/` and `cmake/` directories and the `modules/*` subdirectories listed in [.gitlinks](.gitlinks) (Catch2, CppBenchmark, CppCommon, cpp-optparse, plus the build/cmake script repos) are external repos pulled in by the [gil](https://github.com/chronoxor/gil) tool — they are not committed and `git status` will look "clean" while they are absent. Without them CMake will not configure. After a fresh clone:

```shell
pip3 install gil
gil update
```

Linux additionally needs `sudo apt-get install -y binutils-dev uuid-dev` (CI also installs `libssl-dev`).

## Build, test, run

All builds go through the platform scripts in `build/` (sourced from CppBuildScripts via gil). They invoke CMake, build every target, and `install` everything into `bin/` at the repo root. There is no in-tree configure step — always go through these scripts:

| Platform | Command |
| --- | --- |
| Linux / macOS / Cygwin / MSYS2 | `cd build && ./unix.sh` (or `unix.bat` on Windows shells) |
| Windows MinGW | `cd build && mingw.bat` |
| Windows Visual Studio | `cd build && vs.bat` |

After a successful build, executables live in `bin/`:

- `cpptrader-tests` — Catch2 test runner. Run all: `./bin/cpptrader-tests --durations yes --order lex` (also wired up as `ctest` target `cpptrader-tests`). Single test: pass the test name or tag, e.g. `./bin/cpptrader-tests "[Matching]"` or `./bin/cpptrader-tests "Automatic matching - aggressive limit order"`.
- `cpptrader-example-*` — interactive REPL examples (`itch_handler`, `market_manager`, `matching_engine`).
- `cpptrader-performance-*` — CppBenchmark micro-benchmarks. They read an ITCH file from stdin: `./bin/cpptrader-performance-itch_handler < 01302017.NASDAQ_ITCH50`. The `matching_engine` benchmark reads scenario scripts from `tools/matching/scenario-*.txt`.

Doxygen docs (target `doxygen`) build from `documents/Doxyfile` when Doxygen is installed.

## Architecture

The library is header-heavy C++ (sources in `include/trader/`, `.inl` files for inline definitions, plus a few `.cpp` in `source/trader/`) compiled into a single static lib `cpptrader` that depends on `cppcommon` (from `modules/CppCommon`). Two top-level namespaces under `CppTrader::`:

### `CppTrader::Matching` ([include/trader/matching/](include/trader/matching/))

Single-threaded order matching engine. The dependency chain is `MarketManager` → `OrderBook` → `Level` → `Order`/`Symbol`. Key invariants:

- **`MarketManager`** ([market_manager.h](include/trader/matching/market_manager.h)) owns symbols (vector indexed by symbol id), order books (vector indexed by symbol id), and a global orders `HashMap<uint64_t, OrderNode*, FastHash>` keyed by order id. It is the only public surface for mutating market state (`AddOrder`, `ReduceOrder`, `ModifyOrder`, `MitigateOrder`, `ReplaceOrder`, `DeleteOrder`, `ExecuteOrder`). Not thread-safe.
- **Matching modes**: `EnableMatching()` makes every mutation immediately match against the book; otherwise `Match()` must be called explicitly. Prefer the automatic mode in examples and tests.
- **`OrderBook`** ([order_book.h](include/trader/matching/order_book.h)) holds six AVL trees (`CppCommon::BinTreeAVL`) — bids, asks, buy/sell stops, trailing buy/sell stops — plus cached `_best_bid`, `_best_ask`, `_best_*_stop`, and `_last_*_price` pointers. These cached pointers are load-bearing for performance and must be kept in sync whenever a level is added/removed; touching that code path requires care.
- **Order types**: market, limit, stop, stop-limit, trailing stop, trailing stop-limit, plus IOC/FOK/AON time-in-force and hidden/iceberg orders. See enums in [order.h](include/trader/matching/order.h).
- **`MarketHandler`** ([market_handler.h](include/trader/matching/market_handler.h)) is a virtual callback interface (`onAddSymbol`, `onAddOrder`, `onUpdateLevel`, `onExecuteOrder`, …). Consumers subclass it and pass an instance to the `MarketManager` constructor; the default handler is a no-op.
- **Pool allocators**: orders, price levels, and order books are allocated from `CppCommon::PoolAllocator` instances on the manager. Lifetimes are tied to the manager — never `delete` nodes returned from public getters.

### `CppTrader::ITCH` ([include/trader/providers/nasdaq/itch_handler.h](include/trader/providers/nasdaq/itch_handler.h))

Push-style parser for the NASDAQ ITCH 5.0 binary feed. Feed bytes through `ITCHHandler::Process(buffer, size)`; it dispatches each parsed message to a virtual `onMessage(...)` overload per ITCH message type. The handler is independent of the matching engine — `performance/market_manager.cpp` shows the typical wiring (subclass `ITCHHandler`, translate ITCH messages to `MarketManager` calls).

### Performance variants

[performance/market_manager.cpp](performance/market_manager.cpp) is the reference baseline. The `_optimized` and `_optimized_aggressive` variants in the same directory keep the same external behavior but swap data structures (sorted arrays in place of trees, dropped order linked lists, packed price encoding) for throughput. When changing the core engine, re-run all three benchmarks against the same ITCH capture before claiming a perf win — the optimized variants exist precisely because the tradeoffs are non-obvious.

## Conventions

- C++ headers carry a Doxygen file block (`\file`, `\brief`, `\author`, `\date`, `\copyright MIT License`) — match this style when adding new headers.
- Public headers go in `include/trader/...`; matching `.inl` holds template/inline definitions; `.cpp` (when needed) goes in `source/trader/...` mirroring the include path.
- New examples drop a single `.cpp` in `examples/`; new benchmarks drop a single `.cpp` in `performance/`; both are auto-globbed by CMake at configure time and produce `cpptrader-example-<name>` / `cpptrader-performance-<name>` targets — re-run CMake after adding files.
- Tests are Catch2; add new `.cpp` files to `tests/` and they're picked up automatically.
