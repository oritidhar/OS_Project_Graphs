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

### Milestone 5 – Autonomous IPC travelers
```bash
make milestone5
./sim <input_file>
```
Example:
```bash
./sim assets/samples/test_m5.txt
```

### Milestone 6 – Node mutual exclusion
```bash
make milestone6
./sim <input_file>
```
Example:
```bash
./sim assets/samples/test_m6_two_conflict.txt
```

### Milestone 7 - Scheduling Algorithms
```bash
make milestone7
./sim -schd fcfs demo_schedulers.txt
./sim -schd sjf demo_schedulers.txt
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
```
---


### Milestone 6

## Overview
In this milestone, we implemented a node mutual exclusion mechanism. No more than one passenger (process) can occupy a graph node at any given time. Passengers arriving at an occupied node wait outside and enter sequentially.

## Architecture & System Design
* **Node Synchronization:** Managed via critical sections at the node level.
* **Telemetry & IPC Updates:** Extended `IPCMessage` with `waiting_for_node` and `blocked_at_node` flags to notify the parent process.
* **GUI Visualization:** Upgraded Raylib to render occupied nodes (enlarged with a lock icon) and waiting passengers in a structured queue using positional offsets.

### Why POSIX Semaphores over Mutexes? (Design Rationale)
1. **Cross-Process Synchronization:** Travelers run as separate processes (`fork()`) with independent memory. Standard `pthread_mutex` cannot cross process boundaries, whereas POSIX Named Semaphores (`sem_open`) operate at the OS kernel level.
2. **Deadlock Prevention:** Passengers only request **one lock at a time** (the next hop) and release the previous one immediately, mathematically eliminating circular wait conditions.
3. **Starvation Prevention:** Backed by Linux's kernel-level FIFO wait queue for blocked semaphores, ensuring fair, first-come-first-served entry.
4. **Clean Resource Lifecycle:** The parent process ensures proper cleanup by invoking `sem_close()` and `sem_unlink()` during simulation teardown to prevent kernel leaks.

---

## Test Samples (`assets/samples/`)
1. `test_m6_single.txt` – Single traveler baseline (no contention).
2. `test_m6_two_conflict.txt` – Direct contention (The Train Effect). Two travelers, identical path; each trails exactly 1 node behind the other.
3. `test_m6_src_eq_dst.txt` – Edge case where `src == dst` (path length 1), locking a single node and forcing cross-traffic to wait outside.
4. `test_m6_same_start.txt` – 4 travelers all starting at the same node.
5. `test_m6_chain.txt` – 4 travelers with chain-staggered, converging paths.
6. `test_m6_complex.txt` – Heavy stress test: 6 concurrent travelers through two central bottlenecks (nodes 5 and 7).

---

## Compilation & Execution

### 1. Build the Project
Navigate to the build directory, clear cache, and compile the target:
```bash
cd build
rm -rf *
cmake ..
make sim_m6
```

### Milestone 7 - Scheduling Algorithms

## Overview
Milestone 7 adds selectable scheduling for travelers waiting at graph nodes.
The selected scheduler is shown in the GUI as `Scheduler: FCFS` or
`Scheduler: SJF`, and each node shows its current waiting count as
`Waiting: x0`, `Waiting: x1`, and so on.

## Compilation & Execution
```bash
make milestone7
./sim -schd fcfs demo_schedulers.txt
./sim -schd sjf demo_schedulers.txt
```

## Algorithms
FCFS - first traveler that arrives to a node enters first.

SJF - traveler with the shortest remaining path enters first.

## Demo Input
`demo_schedulers.txt` contains three travelers starting at the same node:
short, medium, and long routes. The long traveler appears first in the input,
so FCFS can let the long traveler wait ahead of shorter travelers, while SJF
prioritizes the shorter remaining path.

## Behavior Comparison
| Algorithm | Behavior | Expected result |
| --- | --- | --- |
| FCFS | Arrival order | The long traveler can enter before the short traveler if it arrived first |
| SJF | `path_remaining` | The short traveler gets priority at the node |

## Waiting Time Comparison
Measured on `demo_schedulers.txt` (3 travelers: 0→2 short, 0→4 medium, 0→7 long).
All three request node 0 nearly simultaneously; the long traveler was granted first
in both runs because it arrived a fraction before the others enqueued.

| Algorithm | Short traveler (0→2) | Medium traveler (0→4) | Long traveler (0→7) |
| --- | --- | --- | --- |
| FCFS | ~2.05s (node 0: 2.02s + node 2: 0.03s) | ~1.02s (node 0) | 0s |
| SJF  | ~1.02s (node 0) | ~2.06s (node 0: 2.03s + nodes 1,2: 0.03s) | 0s |

**Conclusion:** SJF cut the short traveler's wait by ~1s compared to FCFS, at the cost
of making the medium traveler wait ~1s longer. The long traveler was unaffected in both
runs because it reached node 0 before the queue formed.

## Project Structure
```bash
src/core/   – graph representation, Dijkstra, min-heap
src/io/     – file parsing and input validation
src/gui/    – raylib rendering, layout, arrow drawing
include/    – all header files (mirrors src/ structure)
tests/      – test input files
assets/     – sample graphs
```
