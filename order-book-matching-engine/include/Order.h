#ifndef ORDER_H
#define ORDER_H

#include <cstdint>

enum class Side { Buy, Sell };

struct Order {
    int id;
    Side side;
    double price;
    int quantity;
    long timestamp; // used to break ties at the same price (time priority)

    Order(int id_, Side side_, double price_, int quantity_, long timestamp_)
        : id(id_), side(side_), price(price_), quantity(quantity_), timestamp(timestamp_) {}
};

struct Trade {
    int buyOrderId;
    int sellOrderId;
    double price;
    int quantity;
};

#endif // ORDER_H
