cmake_minimum_required(VERSION 3.20)

project(Console2048 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


include(FetchContent)

set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    fmt
    URL https://github.com/fmtlib/fmt/archive/refs/tags/11.1.4.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(fmt)

add_executable(Console2048
    src/main.cpp
    src/Game.cpp
    src/Board.cpp
    include/Game.hpp
    include/Board.hpp
)

target_include_directories(Console2048 PRIVATE include)
target_link_libraries(Console2048 PRIVATE fmt::fmt)

if(MSVC)
    target_compile_options(Console2048 PRIVATE /W4 /permissive-)
else()
    target_compile_options(Console2048 PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(MSVC)
    target_compile_options(Console2048 PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(Console2048 PRIVATE -Wall -Wextra -Wpedantic)
endif()