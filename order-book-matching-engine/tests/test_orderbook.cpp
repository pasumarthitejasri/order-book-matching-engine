/*
 * Unit tests for OrderBook.
 * Lightweight, dependency-free test runner (plain asserts) so the project
 * builds with nothing beyond the standard library.
 *
 * Run: g++ -std=c++17 -Iinclude src/OrderBook.cpp tests/test_orderbook.cpp -o test_orderbook && ./test_orderbook
 */

#include <cassert>
#include <iostream>

#include "OrderBook.h"

static int testsRun = 0;

#define RUN_TEST(fn)                    \
    do {                                 \
        fn();                            \
        testsRun++;                      \
        std::cout << #fn << " passed\n"; \
    } while (0)

void test_no_match_when_book_empty() {
    OrderBook book;
    auto trades = book.addOrder(Order(1, Side::Buy, 100.0, 10, 0));
    assert(trades.empty());
    assert(book.bestBid() == 100.0);
    assert(book.bestAsk() == -1.0);
}

void test_exact_match_full_fill() {
    OrderBook book;
    book.addOrder(Order(1, Side::Sell, 100.0, 10, 0));
    auto trades = book.addOrder(Order(2, Side::Buy, 100.0, 10, 1));

    assert(trades.size() == 1);
    assert(trades[0].price == 100.0);
    assert(trades[0].quantity == 10);
    assert(book.bestAsk() == -1.0); // sell order fully consumed
    assert(book.bestBid() == -1.0); // buy order fully consumed too
}

void test_partial_fill_leaves_remainder_resting() {
    OrderBook book;
    book.addOrder(Order(1, Side::Sell, 100.0, 5, 0));
    auto trades = book.addOrder(Order(2, Side::Buy, 100.0, 12, 1));

    assert(trades.size() == 1);
    assert(trades[0].quantity == 5);
    assert(book.bestAsk() == -1.0);   // sell order fully consumed
    assert(book.bestBid() == 100.0);  // buyer's remaining 7 units now rest
    assert(book.depthAt(Side::Buy, 100.0) == 7);
}

void test_price_priority_best_price_matches_first() {
    OrderBook book;
    book.addOrder(Order(1, Side::Sell, 102.0, 10, 0));
    book.addOrder(Order(2, Side::Sell, 100.0, 10, 1)); // cheaper -> should match first

    auto trades = book.addOrder(Order(3, Side::Buy, 105.0, 10, 2));
    assert(trades.size() == 1);
    assert(trades[0].sellOrderId == 2); // the cheaper resting order matched
    assert(trades[0].price == 100.0);
}

void test_time_priority_at_same_price() {
    OrderBook book;
    book.addOrder(Order(1, Side::Sell, 100.0, 5, 0)); // arrives first
    book.addOrder(Order(2, Side::Sell, 100.0, 5, 1)); // arrives second, same price

    auto trades = book.addOrder(Order(3, Side::Buy, 100.0, 5, 2));
    assert(trades.size() == 1);
    assert(trades[0].sellOrderId == 1); // earlier order at the same price matches first
}

void test_no_match_when_price_does_not_cross() {
    OrderBook book;
    book.addOrder(Order(1, Side::Sell, 105.0, 10, 0));
    auto trades = book.addOrder(Order(2, Side::Buy, 100.0, 10, 1)); // buyer won't pay 105

    assert(trades.empty());
    assert(book.bestBid() == 100.0);
    assert(book.bestAsk() == 105.0);
}

void test_cancel_removes_resting_order() {
    OrderBook book;
    book.addOrder(Order(1, Side::Buy, 99.0, 10, 0));
    assert(book.orderCount() == 1);

    bool cancelled = book.cancelOrder(1);
    assert(cancelled);
    assert(book.orderCount() == 0);
    assert(book.bestBid() == -1.0);
}

void test_cancel_nonexistent_order_returns_false() {
    OrderBook book;
    assert(book.cancelOrder(999) == false);
}

void test_multiple_price_levels_are_walked_in_order() {
    OrderBook book;
    book.addOrder(Order(1, Side::Sell, 100.0, 5, 0));
    book.addOrder(Order(2, Side::Sell, 101.0, 5, 1));
    book.addOrder(Order(3, Side::Sell, 102.0, 5, 2));

    // A large buy order should sweep through all three levels in price order.
    auto trades = book.addOrder(Order(4, Side::Buy, 102.0, 15, 3));
    assert(trades.size() == 3);
    assert(trades[0].price == 100.0);
    assert(trades[1].price == 101.0);
    assert(trades[2].price == 102.0);
}

int main() {
    RUN_TEST(test_no_match_when_book_empty);
    RUN_TEST(test_exact_match_full_fill);
    RUN_TEST(test_partial_fill_leaves_remainder_resting);
    RUN_TEST(test_price_priority_best_price_matches_first);
    RUN_TEST(test_time_priority_at_same_price);
    RUN_TEST(test_no_match_when_price_does_not_cross);
    RUN_TEST(test_cancel_removes_resting_order);
    RUN_TEST(test_cancel_nonexistent_order_returns_false);
    RUN_TEST(test_multiple_price_levels_are_walked_in_order);

    std::cout << "\nAll " << testsRun << " tests passed.\n";
    return 0;
}
