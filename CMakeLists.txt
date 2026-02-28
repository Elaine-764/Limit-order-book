cmake_minimum_required(VERSION 3.16)
project(lob)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The core library: matching engine, no networking
add_library(lob_core
    src/order_book.cc
    src/price_level.cc
)
target_include_directories(lob_core PUBLIC ${CMAKE_SOURCE_DIR}/include)

# The server executable
add_executable(lob_server src/server.cc)
target_link_libraries(lob_server PRIVATE lob_core)

# Tests
add_executable(tests
    tests/test_add_order.cc
    tests/test_cancel_order.cc
    tests/test_matching.cc
)
target_link_libraries(tests PRIVATE lob_core)