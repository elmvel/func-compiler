cmake_minimum_required(VERSION 3.20)

project(
    func-compiler
    VERSION 0.1.0
    DESCRIPTION "A brief implementation of a unique functional language compiler to study backend optimization."
    LANGUAGES CXX C
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

add_executable(fc src/compiler/main.cc
                  src/compiler/file_reading.cc
                  src/compiler/frontend/scanner.cc
                  src/compiler/frontend/scanner_debug.cc
                  src/compiler/frontend/parser.cc
                  src/compiler/frontend/symtab.cc
                  src/compiler/frontend/sema.cc
                  src/compiler/frontend/types.cc
                  src/compiler/backend/lambda_debug.cc
                  src/compiler/backend/high_to_elc.cc
                  src/compiler/backend/lambda_lifting.cc
                  src/compiler/backend/ginstr.cc
                  src/compiler/backend/sc_to_gcode.cc
                  src/compiler/backend/gcode_to_c.cc)

target_link_libraries(fc fmt::fmt)

add_executable(runtime-probe src/runtime-probe/main.c)
