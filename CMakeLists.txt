cmake_minimum_required(VERSION 3.16)
project(lob)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Curses REQUIRED)

add_library(lob_core
    src/order_book.cc
    src/price_level.cc
    src/utilities.cc
    src/feed_handler.cc
    src/exchange.cc
)
target_include_directories(lob_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(lob_core PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
)

add_executable(lob_server src/server.cc)
target_link_libraries(lob_server PRIVATE lob_core)

add_executable(lob_client src/client.cc)
target_link_libraries(lob_client PRIVATE lob_core ${CURSES_LIBRARIES})
target_include_directories(lob_client PRIVATE ${CURSES_INCLUDE_DIRS})

# add_executable(tests
#     tests/test_add_order.cc
#     tests/test_cancel_order.cc
#     tests/test_matching.cc
# )
# target_link_libraries(tests PRIVATE lob_core)