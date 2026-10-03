cmake_minimum_required(VERSION 3.24)
project(aob_network LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(aob_network INTERFACE)
target_include_directories(aob_network INTERFACE include)

enable_testing()
add_executable(aob_network_test tests/network_test.cpp)
target_link_libraries(aob_network_test PRIVATE aob_network)
add_test(NAME network COMMAND aob_network_test)
