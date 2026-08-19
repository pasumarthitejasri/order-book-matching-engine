#include "OrderBook.h"

std::vector<Trade> OrderBook::addOrder(const Order& order) {
    if (order.side == Side::Buy) {
        return matchBuyOrder(order);
    }
    return matchSellOrder(order);
}

std::vector<Trade> OrderBook::matchBuyOrder(Order buyOrder) {
    std::vector<Trade> trades;

    // Match against resting sell orders as long as the best ask is
    // affordable (<= the buy order's price) and the buy order still has
    // quantity left.
    while (buyOrder.quantity > 0 && !asks_.empty()) {
        auto bestLevelIt = asks_.begin();
        double bestAskPrice = bestLevelIt->first;
        if (bestAskPrice > buyOrder.price) {
            break; // best ask is more expensive than the buyer is willing to pay
        }

        auto& ordersAtLevel = bestLevelIt->second;
        Order& resting = ordersAtLevel.front(); // earliest order at this price = time priority

        int tradedQty = std::min(buyOrder.quantity, resting.quantity);
        trades.push_back(Trade{buyOrder.id, resting.id, bestAskPrice, tradedQty});

        buyOrder.quantity -= tradedQty;
        resting.quantity -= tradedQty;

        if (resting.quantity == 0) {
            orderIndex_.erase(resting.id);
            ordersAtLevel.pop_front();
            if (ordersAtLevel.empty()) {
                asks_.erase(bestLevelIt);
            }
        }
    }

    if (buyOrder.quantity > 0) {
        insertResting(buyOrder); // remainder rests on the book
    }
    return trades;
}

std::vector<Trade> OrderBook::matchSellOrder(Order sellOrder) {
    std::vector<Trade> trades;

    while (sellOrder.quantity > 0 && !bids_.empty()) {
        auto bestLevelIt = bids_.begin();
        double bestBidPrice = bestLevelIt->first;
        if (bestBidPrice < sellOrder.price) {
            break; // best bid doesn't meet the seller's asking price
        }

        auto& ordersAtLevel = bestLevelIt->second;
        Order& resting = ordersAtLevel.front();

        int tradedQty = std::min(sellOrder.quantity, resting.quantity);
        trades.push_back(Trade{resting.id, sellOrder.id, bestBidPrice, tradedQty});

        sellOrder.quantity -= tradedQty;
        resting.quantity -= tradedQty;

        if (resting.quantity == 0) {
            orderIndex_.erase(resting.id);
            ordersAtLevel.pop_front();
            if (ordersAtLevel.empty()) {
                bids_.erase(bestLevelIt);
            }
        }
    }

    if (sellOrder.quantity > 0) {
        insertResting(sellOrder);
    }
    return trades;
}

void OrderBook::insertResting(const Order& order) {
    if (order.side == Side::Buy) {
        bids_[order.price].push_back(order);
    } else {
        asks_[order.price].push_back(order);
    }
    orderIndex_[order.id] = OrderLocation{order.side, order.price};
}

bool OrderBook::cancelOrder(int orderId) {
    auto it = orderIndex_.find(orderId);
    if (it == orderIndex_.end()) {
        return false;
    }

    const OrderLocation& loc = it->second;
    if (loc.side == Side::Buy) {
        auto levelIt = bids_.find(loc.price);
        if (levelIt != bids_.end()) {
            auto& orders = levelIt->second;
            orders.remove_if([orderId](const Order& o) { return o.id == orderId; });
            if (orders.empty()) bids_.erase(levelIt);
        }
    } else {
        auto levelIt = asks_.find(loc.price);
        if (levelIt != asks_.end()) {
            auto& orders = levelIt->second;
            orders.remove_if([orderId](const Order& o) { return o.id == orderId; });
            if (orders.empty()) asks_.erase(levelIt);
        }
    }

    orderIndex_.erase(it);
    return true;
}

double OrderBook::bestBid() const {
    return bids_.empty() ? -1.0 : bids_.begin()->first;
}

double OrderBook::bestAsk() const {
    return asks_.empty() ? -1.0 : asks_.begin()->first;
}

int OrderBook::depthAt(Side side, double price) const {
    if (side == Side::Buy) {
        auto it = bids_.find(price);
        if (it == bids_.end()) return 0;
        int total = 0;
        for (const auto& o : it->second) total += o.quantity;
        return total;
    } else {
        auto it = asks_.find(price);
        if (it == asks_.end()) return 0;
        int total = 0;
        for (const auto& o : it->second) total += o.quantity;
        return total;
    }
}
