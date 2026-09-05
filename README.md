# Limit Order Book Engine

A C++20 exchange-style limit order book and matching engine implementing price-time priority, common order types, fill policies, and time-in-force semantics.

> **Project status:** A new v2 architecture is currently under active development.
>
> **This `main` branch contains the stable, working v1 implementation.**
> The new implementation is being developed separately on [`v2-development`](../../tree/v2-development).
>
> **[→ View the new v2 implementation](../../tree/v2-development)**

## Overview

This project implements an exchange-style limit order book and matching engine in modern C++.

The original v1 implementation provides a working baseline for order management and matching. The v2 implementation is a substantial architectural refactor focused on improving order storage, cancellation efficiency, testability, and overall system design.

## Implemented in v1

* Limit orders
* Market orders
* Stop orders
* Stop-limit orders
* Price-time priority
* Fill-or-kill (FOK) orders
* All-or-none (AON) orders
* Good-for-day (DAY) time-in-force
* Good-til-cancelled (GTC) time-in-force
* Bid/ask price levels
* Order cancellation and lookup

## V2 Development

The v2 implementation is being rebuilt around more efficient order-management data structures.

Current architectural work includes:

* Ordered price levels for bid/ask books
* Linked-list order storage within price levels
* Iterator-backed order locations
* Hash-based order ID indexing
* Dedicated stop-order storage
* Unit testing for core order-book components
* Cleaner separation between order management and matching logic

The goal is to support:

* `O(log n)` price-level operations
* `O(1)` average-case order lookup
* `O(1)` average-case order cancellation once the order location is indexed

The v2 matching engine and additional tests are currently under development.

## Repository Structure

The stable `main` branch and active `v2-development` branch represent two stages of the project.

```text
main
└── Stable v1 implementation

v2-development
└── Refactored v2 implementation
```

Once complete, v2 will replace v1 as the primary implementation.

## Why v2?

The v1 implementation uses a simpler architecture intended to establish
correct matching and order-management semantics.

The v2 refactor addresses limitations in the original design by introducing
explicit data structures for price levels, order locations, and stop orders.
The primary goals are improved cancellation complexity, clearer ownership and
responsibilities, stronger unit-test coverage, and a foundation for
performance benchmarking.

## Building

### Requirements

* C++20-compatible compiler
* CMake

### Build

```bash
cmake -S . -B build
cmake --build build
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```

## Design Goals

The project is intended as a learning-focused implementation of exchange-style order-book infrastructure, with particular emphasis on:

* Data-structure selection
* Matching semantics
* Algorithmic complexity
* Order lifecycle management
* Testability
* Performance measurement

The v2 refactor is being developed incrementally so that architectural changes can be tested against a stable working baseline.

## Development History

### v1 — Stable

The original implementation established the core order-book and matching-engine functionality.

### v2 — In Development

The current refactor focuses on improving the underlying data structures and order-management architecture before extending and benchmarking the matching engine.

---

## License

MIT License. See [LICENSE](LICENSE) for details.
