cmake_minimum_required(VERSION 3.20)
project(aob_protocol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(aob_protocol INTERFACE)
target_include_directories(aob_protocol INTERFACE include)
