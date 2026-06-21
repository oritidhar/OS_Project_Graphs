# Architecture Map

A single place to find everything in the project. The program simulates
travelers moving across a directed weighted graph, evolving across 7 milestones
from a CLI Dijkstra tool to a multi-process, synchronized, scheduler-driven GUI.

---

## 1. Directory layout

```
OS_Project_Graphs/
├── CMakeLists.txt          # defines every binary target (source of truth for builds)
├── Makefile                # milestone1..7 wrappers around cmake + copy to ./sim
├── README.md               # per-milestone build/run + scheduler comparison
├── ARCHITECTURE.md         # this file
│
├── include/                # public headers (mirrors src/ layout)
│   ├── core/               # algorithms, processes, IPC, sync, scheduling
│   ├── gui/                # raylib drawing helpers
│   └── io/                 # file parsing
│
├── src/
│   ├── main.c              # M1 CLI entry point (./dijkstra)
│   ├── core/               # graph, dijkstra, minHeap, process_mgr, ipc, sync, schedulers, animator
│   ├── gui/                # one gui_main_*.c entry point per GUI milestone + drawing modules
│   └── io/                 # file_parser.c
│
├── assets/samples/         # example graph + traveler input files
├── tests/                  # test harnesses + fixtures (see §6)
└── build/                  # cmake build dir (generated; gitignored)
```

The `include/` tree mirrors `src/` one-to-one for every module that has a public
header. `#include "core/foo.h"` resolves via `include_directories(include)` in
CMake.

---

## 2. Milestone → binary → entry point

| Milestone | Binary (cmake target) | Entry point | Run command |
|-----------|-----------------------|-------------|-------------|
| 1 | `dijkstra` | [src/main.c](src/main.c) | `./dijkstra <file>` |
| 2 | `sim_static` | [src/gui/gui_main_static.c](src/gui/gui_main_static.c) | `./sim_static <file>` |
| 3 | `sim` | [src/gui/gui_main.c](src/gui/gui_main.c) | `./sim <file>` |
| 4 | `sim_m4` | [src/gui/gui_main_m4.c](src/gui/gui_main_m4.c) | `./sim <file>` |
| 5 | `sim_m5` | [src/gui/gui_main_m5.c](src/gui/gui_main_m5.c) | `./sim <file>` |
| 6 | `sim_m6` | [src/gui/gui_main_m5.c](src/gui/gui_main_m5.c) | `./sim <file>` |
| 7 | `sim_m7` | [src/gui/gui_main_m5.c](src/gui/gui_main_m5.c) | `./sim -schd fcfs\|sjf <file>` |

`gui_main_m5.c` is the shared parent/GUI driver for M5, M6 and M7 — the same
source compiled into three targets as the feature set grows (IPC → node sync →
scheduler selection). Each `make milestoneN` builds the matching cmake target
and copies it to `./sim` (or `./dijkstra` / `./sim_static`).

---

## 3. What each module does

### core/ — logic, processes, coordination
| File | Responsibility |
|------|----------------|
| [graph.c](src/core/graph.c) | Adjacency-list graph: create / addEdge / free. Rejects negative weights. |
| [minHeap.c](src/core/minHeap.c) | Binary min-heap (Dijkstra's priority queue) with `pos[]` for O(log n) decreaseKey. |
| [dijkstra.c](src/core/dijkstra.c) | Shortest path. `dijkstra()` prints (CLI); `dijkstra_compute_path()` returns a `PathResult` for the GUI. |
| [process_mgr.c](src/core/process_mgr.c) | fork/wait of traveler children + the per-node request→grant→enter→release protocol (M5-7). |
| [ipc.c](src/core/ipc.c) | Per-child pipe setup + atomic `IPCMessage` send/recv (non-blocking read). |
| [sync.c](src/core/sync.c) | Per-node mutual exclusion via POSIX named semaphores (`/osproj_node_<pid>_<i>`). |
| [scheduler_fcfs.c](src/core/scheduler_fcfs.c) | FCFS queue + the dispatcher (`scheduler_*`) that routes to FCFS or SJF. |
| [scheduler_sjf.c](src/core/scheduler_sjf.c) | SJF: sorted insertion by `path_remaining`, FIFO tie-break. |
| [animator.c](src/core/animator.c) | Per-traveler SLIDE/WAIT animation state machine driven by IPC updates. |

### gui/ — raylib rendering (all drawing happens between Begin/EndDrawing)
| File | Responsibility |
|------|----------------|
| [layout.c](src/gui/layout.c) | Circular screen position for each vertex. |
| [renderer.c](src/gui/renderer.c) | Static graph: header, edges, nodes, shortest-path highlight. |
| [arrow_utils.c](src/gui/arrow_utils.c) | Directed-arrow drawing + weight-label placement. |
| [draw_entity.c](src/gui/draw_entity.c) | Traveler circles; moving vs waiting states; locked-node rings; legend. |
| [ui_controls.c](src/gui/ui_controls.c) | Play/stop button, arrival banner, "READY" indicator. |

### io/
| File | Responsibility |
|------|----------------|
| [file_parser.c](src/io/file_parser.c) | `parseGraph()` (graph + query) and `parseGraphWithTravelers()` (graph + travelers section). |

---

## 4. Runtime data flow

### M1 (CLI)
```
main.c → parseGraph() → dijkstra() → prints path + weight → freeGraph()
```

### M2–M3 (single-process GUI)
```
gui_main(_static).c
   → parseGraph() → dijkstra_compute_path()
   → raylib loop: renderer (static graph + highlight)
                + animator + draw_entity (M3 only)
```

### M5–M7 (multi-process)
```
PARENT (gui_main_m5.c)                 CHILD per traveler (process_mgr.c)
─────────────────────                  ──────────────────────────────────
parse + fork children      ── fork ──▶ dijkstra_compute_path() (own path)
                                       for each node on the path:
ipc_recv() each frame  ◀── pipe ─────    send "waiting" (arrival_time, path_remaining)
scheduler picks next                     sigwait(SIGUSR1)        ◀── grant ── parent
grant entry (SIGUSR1)  ─── signal ──▶    node_lock() ; send "entered"
update animation                         hold node 1s
draw GUI                                 node_unlock(); send "released"
                                       send "finished"
```

Key invariant (M6): **at most one traveler inside any node**, enforced by the
node semaphore in `sync.c`; verified independently by the test harness via
shared-memory occupancy counters. M7 only changes *which* waiting traveler the
parent grants next (FCFS vs SJF) — it does not weaken the invariant.

### The `IPCMessage` (the contract between child and parent)
Defined in [include/core/ipc.h](include/core/ipc.h). 8 fields:
`pid, arrival_time, path_remaining, current_node, next_node, finished,
waiting_for_node, blocked_at_node`. **Always construct it with designated
initializers** (`.field = ...`) — positional initializers silently break when
fields are added (this caused a real test bug; see git history).

---

## 5. "I want to change X — where do I look?"

| Goal | File(s) |
|------|---------|
| Input file format / parsing rules | [file_parser.c](src/io/file_parser.c) |
| Shortest-path algorithm | [dijkstra.c](src/core/dijkstra.c) + [minHeap.c](src/core/minHeap.c) |
| Add a new scheduling algorithm | [scheduler.h](include/core/scheduler.h) + new `scheduler_*.c`; wire into the dispatcher in [scheduler_fcfs.c](src/core/scheduler_fcfs.c) and the `-schd` parse in [gui_main_m5.c](src/gui/gui_main_m5.c) |
| Node locking / mutual exclusion | [sync.c](src/core/sync.c) |
| Child↔parent message shape | [ipc.h](include/core/ipc.h) + [ipc.c](src/core/ipc.c) |
| Fork / signal / per-node protocol | [process_mgr.c](src/core/process_mgr.c) |
| How travelers move/animate | [animator.c](src/core/animator.c) |
| Node/edge/arrow appearance | [renderer.c](src/gui/renderer.c), [arrow_utils.c](src/gui/arrow_utils.c) |
| Traveler dots / waiting visuals | [draw_entity.c](src/gui/draw_entity.c) |
| Buttons, banners, on-screen text | [ui_controls.c](src/gui/ui_controls.c) |
| Node screen positions | [layout.c](src/gui/layout.c) |
| Add a file to a build / new target | [CMakeLists.txt](CMakeLists.txt) (then [Makefile](Makefile) if a new milestone) |

---

## 6. Tests

| File | Covers | Run |
|------|--------|-----|
| [tests/dijkstra/](tests/dijkstra/) | M1 correctness + invalid-input fixtures | `./dijkstra tests/dijkstra/<f>.txt` |
| [tests/test_m6_sync.c](tests/test_m6_sync.c) | M6 mutual exclusion / no starvation / waiting events (headless, no raylib) | see below |
| [tests/test_m7_scheduler.c](tests/test_m7_scheduler.c) | M7 FCFS vs SJF ordering (headless) | see below |
| [assets/samples/](assets/samples/) | graph + traveler input fixtures | — |

Build & run the headless harnesses (binaries are gitignored):

```bash
# M6 sync harness
gcc -std=c99 -D_GNU_SOURCE -Iinclude -o tests/run_m6_tests tests/test_m6_sync.c \
    src/core/graph.c src/core/dijkstra.c src/core/minHeap.c src/core/ipc.c src/core/sync.c \
    -lpthread -lrt -lm
./tests/run_m6_tests

# M7 scheduler test
gcc -std=c99 -Iinclude -Ibuild/_deps/raylib-build/raylib/include -o tests/run_m7_tests \
    tests/test_m7_scheduler.c src/core/scheduler_fcfs.c src/core/scheduler_sjf.c
./tests/run_m7_tests
```

GUI binaries need a display; for headless smoke tests use `xvfb-run -a ./sim <file>`.

---

## 7. Build cheatsheet

```bash
make milestone1   # ./dijkstra
make milestone2   # ./sim_static
make milestone3   # ./sim   (single animated traveler)
make milestone4   # ./sim   (multi-process, parent computes paths)
make milestone5   # ./sim   (autonomous children + IPC)
make milestone6   # ./sim   (+ node mutual exclusion)
make milestone7   # ./sim   (+ -schd fcfs|sjf selection)
make clean
```
