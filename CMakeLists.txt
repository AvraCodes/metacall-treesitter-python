cmake_minimum_required(VERSION 3.15)
project(MetaCallTreeSitterPython)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tree-sitter core library
set(TS_LIB_DIR ${CMAKE_SOURCE_DIR}/tree-sitter/lib)

# Tree-sitter Python parser
set(TS_PYTHON_DIR ${CMAKE_SOURCE_DIR}/tree-sitter-python)

# Build tree-sitter library as a static library
add_library(tree-sitter STATIC
    ${TS_LIB_DIR}/src/lib.c
)

target_include_directories(tree-sitter PUBLIC
    ${TS_LIB_DIR}/include
)

# Build the parser executable
add_executable(parser_test
    src/main.cpp
    ${TS_PYTHON_DIR}/src/parser.c
    ${TS_PYTHON_DIR}/src/scanner.c
)

target_include_directories(parser_test PRIVATE
    ${TS_LIB_DIR}/include
    ${TS_PYTHON_DIR}/src
)

target_link_libraries(parser_test PRIVATE
    tree-sitter
)