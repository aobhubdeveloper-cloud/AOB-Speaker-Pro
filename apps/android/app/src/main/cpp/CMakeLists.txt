cmake_minimum_required(VERSION 3.22)
project(aob_audio)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    oboe
    GIT_REPOSITORY https://github.com/google/oboe.git
    GIT_TAG 1.9.3
)
FetchContent_MakeAvailable(oboe)

FetchContent_Declare(
    opus
    GIT_REPOSITORY https://github.com/xiph/opus.git
    GIT_TAG v1.5.2
)

set(OPUS_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(OPUS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(OPUS_INSTALL_CMAKE_CONFIG_MODULE ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(opus)

add_library(aob_audio SHARED native_audio.cpp)

target_include_directories(aob_audio
    PRIVATE
        ../../../../../../core/protocol/include
        ${opus_SOURCE_DIR}/include
        ${opus_BINARY_DIR}/include
)

target_link_libraries(aob_audio
    PRIVATE
        oboe
        opus
        log
        android
)
