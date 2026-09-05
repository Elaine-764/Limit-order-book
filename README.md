# Limit Order Book Engine

A C++20 exchange-style limit order book and matching engine implementing price-time priority, common order types, fill policies, and time-in-force semantics.

> **Project status: v2 actively under development**
>
> This branch contains the new architecture currently being implemented. The original working implementation is preserved on [`main`](../../tree/main) and tagged as [`v1.0.0`](../../releases/tag/v1.0.0).

## Overview

This project implements an exchange-style limit order book and matching engine in modern C++.

The v2 implementation is a ground-up architectural refactor of the original working implementation, with a focus on efficient order management, clean separation of responsibilities, testability, and measurable performance.

## V2 Architecture

The order book maintains separate bid and ask price levels using ordered maps:

```text
OrderBook
├── Bid Price Levels
│   └── PriceLevel
│       └── Orders
├── Ask Price Levels
│   └── PriceLevel
│       └── Orders
└── Order ID Index
    └── OrderLocation
```

Each resting order is stored in a linked list within its price level. An order-ID hash index stores the corresponding price level and list iterator.

This allows cancellation to avoid searching through an entire price level:

```text
Order ID
   ↓
Hash Index
   ↓
OrderLocation
   ↓
PriceLevel + list iterator
   ↓
O(1) average-case erase
```

Price-level operations use ordered maps, providing `O(log n)` lookup/insertion/removal at the price-level layer.

## Supported Order Types

* Limit
* Market
* Stop
* Stop-limit

## Order Policies

* Price-time priority
* Fill-or-kill (FOK)
* All-or-none (AON)
* Good-for-day (DAY)
* Good-til-cancelled (GTC)

## Current Implementation Status

### Complete

* Core order value types
* Order representation
* Bid/ask price-level storage
* Order ID indexing
* Iterator-backed order locations
* Order insertion
* Order cancellation
* Stop-order storage
* Unit tests for core order-book components

### In Progress

* Matching engine
* Market-order matching
* Limit-order matching
* Stop-order triggering
* Stop-limit matching
* Comprehensive matching-engine tests
* Performance benchmarks

## Complexity Goals

| Operation                              | Target Complexity |
| -------------------------------------- | ----------------: |
| Price-level lookup                     |        `O(log n)` |
| Price-level insertion/removal          |        `O(log n)` |
| Order ID lookup                        |    `O(1)` average |
| Order cancellation                     |    `O(1)` average |
| Order modification across price levels |        `O(log n)` |

## Why the Refactor?

The original implementation used sequential order containers within each price level. While effective for basic matching, arbitrary order removal required searching within the container.

The v2 design separates:

1. **Price-level indexing** using ordered maps
2. **Order sequencing** using linked lists
3. **Direct order access** using a hash index

This provides stable iterators for individual orders while preserving price-time ordering within each level.

## Repository Structure

```text
include/       Public headers
src/           Implementation
tests/         Unit tests
CMakeLists.txt Build configuration
README.md      Documentation
```

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

## Progress Tracking

* [x] Refactor project structure
* [x] Implement core order-book data structures
* [x] Implement direct order indexing
* [x] Implement cancellation
* [x] Add unit tests
* [ ] Implement matching engine
* [ ] Add comprehensive matching tests
* [ ] Add benchmarks
* [ ] Analyze performance and data-structure tradeoffs
* [ ] Merge v2 into `main`

## Stable Baseline

The original implementation remains available on [`main`](../../tree/main) and is tagged [`v1.0.0`](../../releases/tag/v1.0.0).

The stable implementation is intentionally preserved as a baseline for comparison and regression testing while v2 is developed.
