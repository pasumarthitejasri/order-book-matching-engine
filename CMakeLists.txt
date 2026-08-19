cmake_minimum_required(VERSION 3.10)
project(OrderBookMatchingEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(orderbook_demo src/OrderBook.cpp src/main.cpp)
add_executable(test_orderbook src/OrderBook.cpp tests/test_orderbook.cpp)
