cmake_minimum_required(VERSION 3.20)

project(
    func-compiler
    VERSION 0.1.0
    DESCRIPTION "A brief implementation of a unique functional language compiler to study backend optimization."
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -ggdb")

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

add_executable(fc src/main.cc
                  src/scanner.cc
                  src/parser.cc
                  src/sema.cc)

target_link_libraries(fc fmt::fmt)
