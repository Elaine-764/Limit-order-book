cmake_minimum_required(VERSION 3.20)
project(LimitOrderBook LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 1. External Dependencies
find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

# 2. Core Exchange Library Target
add_library(lob_core
    src/orderbook.cc
    src/engine.cc
    src/exchange.cc
)

# 🟢 FIXED: Include both 'src' and 'include' directories so paths resolve cleanly everywhere
target_include_directories(lob_core PUBLIC 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(lob_core PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)

# 3. Automatic Third-Party Dependency Fetching
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

FetchContent_Declare(
  google_benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.1
)

# Must be set BEFORE MakeAvailable — these are cache variables benchmark reads at configure time
set(BENCHMARK_ENABLE_TESTING   OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL   OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest google_benchmark)

# 4. Unit Test Suite Config
enable_testing()
add_executable(lob_tests
  tests/unit_tests/test_price_level.cc
  tests/unit_tests/test_order_book.cc
  tests/unit_tests/test_stop_order_book.cc
  tests/unit_tests/test_symbol_table.cc
  tests/unit_tests/test_matching_engine_v2.cc
  # tests/unit_tests/test_exchange_v2.cc
  tests/unit_tests/test_exchange.cc
)
target_link_libraries(lob_tests PRIVATE GTest::gtest_main lob_core)

include(GoogleTest)
gtest_discover_tests(lob_tests)

# 5. Benchmark Performance Config
add_executable(bench tests/benchmarks/benchmark_main.cc)
target_include_directories(bench PRIVATE ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(bench PRIVATE benchmark::benchmark lob_core)