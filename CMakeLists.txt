cmake_minimum_required(VERSION 3.10)
project(OS_Project C)

set(CMAKE_C_STANDARD 99)

# Make all headers under include/ resolvable as "core/graph.h", "io/file_parser.h", etc.
include_directories(include)

add_executable(OS_Project
    src/main.c
    src/core/graph.c
    src/core/dijkstra.c
    src/core/minHeap.c
    src/io/file_parser.c
    src/gui/layout.c
    src/gui/renderer.c
    src/gui/arrow_utils.c
)
