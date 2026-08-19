# Order Book Matching Engine

A simplified limit order book implemented in C++ that matches buy and sell
orders using **price-time priority**, modeling the core matching logic used
by financial exchanges.

## How matching works

- **Price priority** — the best available price always matches first (the
  highest bid for sellers, the lowest ask for buyers).
- **Time priority** — among orders resting at the same price, the order that
  arrived earliest matches first (FIFO within a price level).
- **Partial fills** — if an incoming order is larger than the best resting
  order, it keeps matching against the next-best price level until it's
  fully filled or the book can no longer offer a matching price, at which
  point any unfilled remainder rests on the book.

## Core operations

| Operation | Complexity | Notes |
|---|---|---|
| Add order (matches + rests remainder) | O(log M) | M = number of distinct price levels |
| Cancel order | O(log M) | direct lookup via `orderId -> price` index |
| Best bid / best ask | O(1) | first entry of the sorted price map |

## Data structures used

- `std::map<double, std::list<Order>>` — one sorted map per side (bids
  descending, asks ascending), each price level holding a `std::list` of
  orders in arrival order for O(1) FIFO pop from the front.
- `std::unordered_map<int, OrderLocation>` — O(1) lookup from order ID to
  its side/price, so cancellation doesn't require scanning the book.

## Tech stack

C++17, STL only (`<map>`, `<list>`, `<unordered_map>`) — no external
dependencies.

## Building & running

With CMake:
```bash
mkdir build && cd build
cmake ..
make
./orderbook_demo       # runs the demo scenario
./test_orderbook       # runs the unit tests
```

Or directly with g++:
```bash
g++ -std=c++17 -Iinclude src/OrderBook.cpp src/main.cpp -o demo
./demo

g++ -std=c++17 -Iinclude src/OrderBook.cpp tests/test_orderbook.cpp -o test_orderbook
./test_orderbook
```

## Tests

9 unit tests cover: empty-book behavior, full fills, partial fills, price
priority across multiple levels, time priority at the same price level,
non-crossing orders, cancellation, and sweeping through multiple price
levels in a single order. All tests pass using plain `assert` — no external
test framework required to build.

## Project structure

```
order-book-matching-engine/
├── include/
│   ├── Order.h           # Order & Trade structs
│   └── OrderBook.h        # OrderBook class declaration
├── src/
│   ├── OrderBook.cpp      # matching engine implementation
│   └── main.cpp           # demo program
├── tests/
│   └── test_orderbook.cpp # unit tests
├── CMakeLists.txt
└── README.md
```
