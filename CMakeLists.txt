cmake_minimum_required(VERSION 3.10)
project(OS_Project C)

set(CMAKE_C_STANDARD 99)

include_directories(include)

# הרצה רגילה של דייקסטרה
add_executable(OS_Project
    src/main.c
    src/core/graph.c
    src/core/dijkstra.c
    src/core/minHeap.c
    src/io/file_parser.c
)

# הרצת GUI
add_executable(OS_Project_GUI
    src/gui/gui_main.c
    src/core/graph.c
    src/core/dijkstra.c
    src/core/minHeap.c
    src/io/file_parser.c
    src/gui/layout.c
    src/gui/renderer.c
    src/gui/arrow_utils.c
)

target_link_libraries(OS_Project_GUI raylib m pthread dl rt X11)