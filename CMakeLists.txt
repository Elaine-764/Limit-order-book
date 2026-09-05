cmake_minimum_required(VERSION 3.20)
project(LimitOrderBook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
# find_package(Curses REQUIRED)

add_library(lob_core
    src/orderbook.cc
    src/engine.cc
    src/exchange.cc
    # src/utilities.cc
    # src/feed_handler.cc
    # src/exchange.cc
)
target_include_directories(lob_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(lob_core PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)  # Curses::Curses removed
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(lob_tests
  tests/test_price_level.cc
  tests/test_order_book.cc
  tests/test_stop_order_book.cc
  tests/test_symbol_table.cc
)
target_link_libraries(lob_tests PRIVATE GTest::gtest_main lob_core)

include(GoogleTest)
gtest_discover_tests(lob_tests)

# step 1, configure
#   cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
# step 2, build and compile
#   cmake --build build -j$(nproc)
# step 3, targetted build
#   cmake --build build --target lob_core
#   cmake --build build --target lob_tests
# step 4, run tests
#   ctest --test-dir build --output-on-failure
#   OR
#   ./build/lob_tests

# selectively run tests
    # ./build/lob_tests --gtest_filter=PriceLevelTest.*        # run only one test suite
    # ./build/lob_tests --gtest_filter=*CancelUnknownOrder*    # run one specific test by name pattern
    # ./build/lob_tests --gtest_repeat=100 --gtest_shuffle     # stress-test for order-dependent bugs
    # ./build/lob_tests --gtest_list_tests                     # just list every test, don't run them