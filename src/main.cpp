/*
 * Order Book Matching Engine
 * A simplified limit order book that matches buy and sell orders using
 * price-time priority, modeling core exchange logic.
 *
 * Author: Pasumarthi Tejasri
 * Tech stack: C++, STL (map, list, unordered_map)
 */

#include <iomanip>
#include <iostream>

#include "OrderBook.h"

void printTrades(const std::vector<Trade>& trades) {
    for (const auto& t : trades) {
        std::cout << "  TRADE: buy#" << t.buyOrderId << " x sell#" << t.sellOrderId
                  << " | price=" << t.price << " | qty=" << t.quantity << "\n";
    }
}

int main() {
    OrderBook book;
    long ts = 0;

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "-- Adding resting sell orders --\n";
    book.addOrder(Order(1, Side::Sell, 101.00, 10, ts++));
    book.addOrder(Order(2, Side::Sell, 102.00, 5, ts++));

    std::cout << "-- Adding resting buy orders --\n";
    book.addOrder(Order(3, Side::Buy, 99.00, 8, ts++));

    std::cout << "Best bid: " << book.bestBid() << " | Best ask: " << book.bestAsk() << "\n\n";

    std::cout << "-- Incoming buy order that crosses the spread --\n";
    auto trades = book.addOrder(Order(4, Side::Buy, 101.50, 12, ts++));
    printTrades(trades);
    std::cout << "Best bid: " << book.bestBid() << " | Best ask: " << book.bestAsk() << "\n\n";

    std::cout << "-- Cancelling order #3 --\n";
    bool cancelled = book.cancelOrder(3);
    std::cout << "Cancelled: " << (cancelled ? "yes" : "no") << "\n";

    std::cout << "\nResting orders remaining: " << book.orderCount() << "\n";

    return 0;
}
