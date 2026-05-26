cmake_minimum_required(VERSION 3.11)
project(OS_Project C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

# Try to use local raylib first.
# If raylib is not installed locally, download it with FetchContent.
find_package(raylib QUIET)

if (NOT raylib_FOUND)
    message(STATUS "Local raylib not found. Downloading raylib 5.0...")

    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        5.0
    )
    FetchContent_MakeAvailable(raylib)
else()
    message(STATUS "Using local raylib installation.")
endif()

# Milestone 1 - Dijkstra CLI
add_executable(dijkstra
    src/main.c
    src/core/graph.c
    src/core/dijkstra.c
    src/core/minHeap.c
    src/io/file_parser.c
)

# Milestone 2 - Static GUI
add_executable(sim_static
    src/gui/gui_main_static.c
    src/core/graph.c
    src/core/dijkstra.c
    src/core/minHeap.c
    src/io/file_parser.c
    src/gui/layout.c
    src/gui/renderer.c
    src/gui/arrow_utils.c
)

target_link_libraries(sim_static
    raylib
    m
    pthread
    dl
    rt
    X11
)

# Milestone 3 - GUI with animation
add_executable(sim
    src/gui/gui_main.c
    src/core/graph.c
    src/core/dijkstra.c
    src/core/minHeap.c
    src/io/file_parser.c
    src/core/animator.c
    src/gui/layout.c
    src/gui/renderer.c
    src/gui/arrow_utils.c
    src/gui/draw_entity.c
    src/gui/ui_controls.c
)

target_link_libraries(sim
    raylib
    m
    pthread
    dl
    rt
    X11
)

# Milestone 4 - Multi-process travelers
add_executable(sim_m4
    src/gui/gui_main_m4.c
    src/core/graph.c
    src/core/dijkstra.c
    src/core/minHeap.c
    src/io/file_parser.c
    src/core/animator.c
    src/core/process_mgr.c
    src/gui/layout.c
    src/gui/renderer.c
    src/gui/arrow_utils.c
    src/gui/draw_entity.c
    src/gui/ui_controls.c
)

target_link_libraries(sim_m4
    raylib m pthread dl rt X11
)

# Milestone 5 - IPC autonomous travelers
add_executable(sim_m5
    src/gui/gui_main_m5.c
    src/core/graph.c
    src/core/dijkstra.c
    src/core/minHeap.c
    src/io/file_parser.c
    src/core/animator.c
    src/core/process_mgr.c
    src/core/ipc.c
    src/gui/layout.c
    src/gui/renderer.c
    src/gui/arrow_utils.c
    src/gui/draw_entity.c
    src/gui/ui_controls.c
)

target_link_libraries(sim_m5
    raylib m pthread dl rt X11
)