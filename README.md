# OS_Project_Graphs – Graph Simulation

## Team Members
- Ori Tidhar
- Uriel Dahan
- Oriel Zukerman
- Matanel Rahamim

---

## Build & Run

### Milestone 1 – Dijkstra shortest path
```bash
make milestone1
./dijkstra <input_file>
```
Example:
```bash
./dijkstra tests/dijkstra/test_normal.txt
```

### Milestone 2 – Graph GUI display
```bash
make milestone2
./sim <input_file>
```
Example:
```bash
./sim tests/dijkstra/test_normal.txt
```

### Milestone 3 – Animated path traversal
```bash
make milestone3
./sim <input_file>
```
Example:
```bash
./sim tests/dijkstra/test_normal.txt
```

### Milestone 4 – Multi-traveler simulation
```bash
make milestone4
./sim <input_file>
```

Example:
```bash
make milestone4
./sim tests/milestone4/test_m4.txt
```

### Clean all build artifacts
```bash
make clean
```

---

## Milestone Descriptions

### Milestone 1
Implements a directed weighted graph loaded from a text file, and finds the shortest path between two nodes using Dijkstra's algorithm. Prints the full path and total weight. Handles invalid input, negative weights, unreachable destinations, and same source/destination.

**Input format:**
```
N M        # N nodes, M edges
src dst w  # M edge lines
src dst    # query: source and destination
```

### Milestone 2
Displays the graph loaded from the input file in a raylib GUI window. Each node is drawn as a labeled circle, each edge as a directed arrow with its weight shown. Node positions are calculated automatically. Supports up to 15 nodes. Static display only.

### Milestone 3
Adds animated movement of an entity along the Dijkstra shortest path. A play/stop button controls the animation. The entity moves along each edge in W×300ms (W = edge weight) and waits 1 second at each intermediate node. The shortest path is highlighted in the graph. An arrival message is shown when the destination is reached.

### Milestone 4
Milestone 4
Extends the simulation from a single traveler to multiple travelers moving in parallel using process management (fork). The parent process performs all major tasks: parsing the extended input file, calculating Dijkstra shortest paths for all travelers, and managing the GUI animation. Each traveler is represented by a child process that prints [PID] started and waits for a signal. The parent ensures clean termination of all children using SIGTERM and waitpid().

**Updated Input format:**
The file now includes a dedicated travelers section after the graph definition:
```
# travelers
3          # Number of travelers
0 5        # Source and destination for traveler 1
1 4        # Source and destination for traveler 2
2 3        # Source and destination for traveler 3
```

### Milestone 5

## Overview
In this milestone, we upgraded our graph navigation simulation into a multi-process architecture using **Inter-Process Communication (IPC)**. 
Each traveler now runs within its own dedicated child process (`fork()`) and calculates its optimal path independently. The main process (Parent) orchestrates the simulation, manages the Raylib GUI, and listens asynchronously to updates from all child processes via **Pipes**.

## Architecture & System Design
* **Parent Process:** Handles the Raylib GUI lifecycle, renders the graph, and monitors child processes using a non-blocking `read` loop to prevent interface freezing.
* **Child Processes:** Each traveler runs as a separate process, executes Dijkstra's algorithm, and sends real-time position updates to the parent through its pipe.
* **IPC Channel:** Unix Pipes (`pipe()`) configured to non-blocking mode.

### Why Pipes over Shared Memory? (Design Rationale)
During the design phase, we evaluated different IPC mechanisms and chose **Pipes** rather than **Shared Memory** due to the following structural and architectural benefits:
1. **Unidirectional Data Flow:** Our system has a clear, one-way telemetry stream: child processes (travelers) generate position updates, and the parent process (GUI) consumes them. Pipes naturally fit this producer-consumer model between related processes.
2. **Kernel-Level Synchronization:** Shared memory requires manual and error-prone synchronization using primitives to prevent race conditions (such as a child process overwriting memory while the parent is reading it). Pipes handle queuing and synchronization automatically at the OS kernel level.
3. **Seamless Non-Blocking Polling:** By setting the read-end of the pipes to `O_NONBLOCK` via `fcntl()`, the parent can rapidly poll for new updates from multiple children inside the Raylib rendering loop, ensuring a smooth and responsive GUI without freezing.

---

## Test Samples (`assets/samples/`)
We implemented 4 types of test scenarios to validate the stability of the system:
1. `test_m5.txt` – Standard execution with parallel travelers.
2. `test_m5_single.txt` – Single traveler baseline scenario.
3. `test_m5_same_node.txt` – Edge case where Source equals Destination (checks for immediate exit and prevents infinite loops).
4. `test_m5_load.txt` – Stress test involving 4 parallel travelers with intersecting paths.

---

## Compilation & Execution

### 1. Build the Project
Navigate to the build directory, clear cache, and compile the target:
```bash
cd build
rm -rf *
cmake ..
make sim_m5
---

## Project Structure
```
src/core/   – graph representation, Dijkstra, min-heap
src/io/     – file parsing and input validation
src/gui/    – raylib rendering, layout, arrow drawing
include/    – all header files (mirrors src/ structure)
tests/      – test input files
assets/     – sample graphs
```
