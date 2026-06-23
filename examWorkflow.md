# OS Project - Exam Day Workflow

This document is a practical checklist for working on the OS project during the final exam.

Goal:
Work in a clean and organized way:
1. Open the Ubuntu VM
2. Clone the project from GitHub
3. Create a personal exam branch
4. Find the relevant place in the code
5. Make a small focused change
6. Build and run if possible
7. Commit and push the solution

# 1. Open Ubuntu on the college computer

On Windows:
1. Open VMWare
2. Click Open a virtual machine
3. Navigate to:
This PC / C / Program Files / Virtual Machines / Ubuntu Linux 64-bit
4. Start the virtual machine

# 2. Open Terminal and prepare a working folder

cd ~/Desktop

mkdir exam_project

cd exam_project

# 3. Clone the project from GitHub

git clone https://github.com/oritidhar/OS_Project_Graphs.git

cd OS_Project_Graphs

# 4. Make sure main is updated

git checkout main

git pull origin main

# 5. Create a personal exam branch

Replace ID_NUMBER with your ID number.

If the task is called exam_a:


git checkout -b exam_a/ID_NUMBER


If the task is called exam_b:


git checkout -b exam_b/ID_NUMBER


Example:

git checkout -b exam_a/123456789

Important:
Do not work directly on main.

# 6. Open the project in VSCode

code .

If code . does not work, open VSCode manually and select:
File -> Open Folder -> OS_Project_Graphs

# 7. If Raylib or build dependencies are missing

First, try to build normally.

If the build fails because of Raylib or missing system libraries, run:

sudo apt update

sudo apt install -y build-essential cmake git libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev


Then try building again.

Note:
The project CMake configuration should download Raylib 5.0 automatically if Raylib is not installed locally.

# 8. Build and run commands by milestone

Milestone 1 - Dijkstra CLI:

make milestone1

./dijkstra tests/dijkstra/test_normal.txt

Milestone 2 - Static GUI:

make milestone2

./sim_static tests/dijkstra/test_normal.txt

Milestone 3 - GUI Animation:

make milestone3

./sim tests/dijkstra/test_normal.txt

Milestone 4 - Processes and Signals:

make milestone4

./sim tests/milestone4/test_m4.txt

Milestone 5 - IPC:

make milestone5

./sim tests/milestone4/test_m4.txt

Milestone 6 - Semaphores and Synchronization:

make milestone6

./sim tests/milestone4/test_m4.txt

Milestone 7 - FCFS / SJF Scheduler:

make milestone7

./sim tests/milestone4/test_m4.txt

# 9. What to do when receiving the task

Do not start coding immediately.

Recommended order:
1. Read the task carefully.
2. Identify the relevant milestone.
3. Open ARCHITECTURE.md.
4. Search for keywords from the task.
5. Search inside the code using grep.
6. Find the most relevant source file.
7. Make a small and focused change.
8. Build the relevant milestone.
9. Run the relevant executable if possible.
10. Commit and push.

# 10. Quick code search by topic

## Signals / Processes / fork / kill / SIGUSR1

Useful when the task mentions:
- child process
- signal
- SIGUSR1
- SIGTERM
- kill
- fork
- waitpid
- sleep time

Search commands:

grep -R "SIG" -n src include
grep -R "SIGTERM" -n src include
grep -R "SIGUSR1" -n src include
grep -R "kill" -n src include
grep -R "fork" -n src include
grep -R "waitpid" -n src include
grep -R "sleep" -n src include

Likely relevant files:
src/core/process_mgr.c
include/core/process_mgr.h
src/gui/gui_main_m4.c

Recommended build/run:

make milestone4
./sim tests/milestone4/test_m4.txt

## IPC / Messages between processes

Useful when the task mentions:
- communication between parent and child
- pipes
- read/write
- IPCMessage
- messages

Search commands:

grep -R "IPC" -n src include

grep -R "IPCMessage" -n src include

grep -R "pipe" -n src include

grep -R "read(" -n src include

grep -R "write(" -n src include

grep -R "message" -n src include


Likely relevant files:
src/core/ipc.c
include/core/ipc.h
src/core/process_mgr.c

Recommended build/run:

make milestone5
./sim tests/milestone4/test_m4.txt

## Semaphores / Synchronization / Locks

Useful when the task mentions:
- node locking
- preventing two travelers from entering the same node
- semaphore
- lock
- unlock
- synchronization

Search commands:

grep -R "sem_" -n src include

grep -R "semaphore" -n src include

grep -R "lock" -n src include

grep -R "unlock" -n src include

grep -R "sync" -n src include

Likely relevant files:
src/core/sync.c
include/core/sync.h
src/core/process_mgr.c

Recommended build/run:

make milestone6
./sim tests/milestone4/test_m4.txt

## Scheduler / FCFS / SJF

Useful when the task mentions:
- scheduling
- choosing the next traveler
- FCFS
- SJF
- queue
- shortest job
- shortest path

Search commands:

grep -R "scheduler" -n src include

grep -R "FCFS" -n src include

grep -R "SJF" -n src include

grep -R "queue" -n src include

grep -R "shortest" -n src include

Likely relevant files:
src/core/scheduler_fcfs.c
src/core/scheduler_sjf.c
include/core/scheduler_fcfs.h
include/core/scheduler_sjf.h
src/gui/gui_main_m5.c

Recommended build/run:

make milestone7
./sim tests/milestone4/test_m4.txt

## Parser / Input file

Useful when the task mentions:
- changing the input file format
- loading graph data
- loading travelers
- reading nodes
- reading edges

Search commands:

grep -R "fscanf" -n src include

grep -R "fgets" -n src include

grep -R "parse" -n src include

grep -R "file" -n src include

grep -R "traveler" -n src include

Likely relevant files:

src/io/file_parser.c

include/io/file_parser.h

Recommended build/run depends on the task.

For Dijkstra:

make milestone1
./dijkstra tests/dijkstra/test_normal.txt

For GUI/process-related tasks:

make milestone4
./sim tests/milestone4/test_m4.txt

## GUI / Raylib / Drawing

Useful when the task mentions:
- visual change
- color
- text on screen
- drawing travelers
- drawing nodes
- drawing edges
- labels

Search commands:

grep -R "Draw" -n src include

grep -R "BeginDrawing" -n src include

grep -R "EndDrawing" -n src include

grep -R "raylib" -n src include

grep -R "Text" -n src include

Likely relevant files:
src/gui/renderer.c
src/gui/layout.c
src/gui/draw_entity.c
src/gui/ui_controls.c
src/gui/gui_main.c
src/gui/gui_main_m4.c
src/gui/gui_main_m5.c

Recommended build/run depends on the task.

For basic GUI:

make milestone3
./sim tests/dijkstra/test_normal.txt

For process-related GUI:

make milestone4
./sim tests/milestone4/test_m4.txt

## Dijkstra / Graph

Useful when the task mentions:
- shortest path
- graph logic
- nodes
- edges
- distances
- path calculation

Search commands:

grep -R "dijkstra" -n src include

grep -R "Graph" -n src include

grep -R "Node" -n src include

grep -R "Edge" -n src include

grep -R "distance" -n src include

grep -R "shortest" -n src include

Likely relevant files:
src/core/graph.c
include/core/graph.h
src/core/dijkstra.c
include/core/dijkstra.h
src/core/minHeap.c
include/core/minHeap.h

Recommended build/run:

make milestone1
./dijkstra tests/dijkstra/test_normal.txt

# 11. Example approach for a Signals task

Example task:
Change child process behavior so each child receives SIGUSR1 instead of a signal that kills it. When receiving SIGUSR1, the child should print how long it slept and only then exit.

Approach:
1. Identify the task as related to Milestone 4.
2. Search for signal/process code:

grep -R "SIG" -n src include

grep -R "kill" -n src include

grep -R "sleep" -n src include

grep -R "fork" -n src include

3. Likely files:
src/core/process_mgr.c
include/core/process_mgr.h
src/gui/gui_main_m4.c

4. Find where the parent sends a signal to child processes.
5. Replace the current signal with SIGUSR1.
6. Add or update a SIGUSR1 signal handler in the child process.
7. Make the child print the sleep duration.
8. Exit only after printing.
9. Build and run:

make milestone4
./sim tests/milestone4/test_m4.txt

10. Commit and push.

# 12. Check changes before commit

Check which files changed:

git status

Check the actual code changes:

git diff

If an unrelated file changed by mistake, do not include it in the commit.

# 13. Commit

Add changes:

git add .

If the code was implemented and tested:

git commit -m "exam_a: stages 1,2,3 - implemented and verified task"

If the code is logical but was not tested:

git commit -m "exam_a: stages 1,2 - located relevant code and implemented logical change"

If only the relevant code location was found and implementation was started:

git commit -m "exam_a: stage 1 - located relevant code and started implementation"

If the task is exam_b, replace exam_a with exam_b.

# 14. Push

If the branch is exam_a/ID_NUMBER:

git push -u origin exam_a/ID_NUMBER

If the branch is exam_b/ID_NUMBER:

git push -u origin exam_b/ID_NUMBER

# 15. Verify everything is saved

git status

Expected output:
nothing to commit, working tree clean

Also open GitHub and verify that the branch exists.

# 16. Common Git problems

If git push asks for login:
Log in to GitHub in the browser or use a GitHub token.

If there are permission errors:
Make sure the GitHub user has access to the repository.

If the branch already exists:

git checkout exam_a/ID_NUMBER
git pull origin exam_a/ID_NUMBER

To check the current branch:

git branch

The active branch is marked with *.

# 17. Final short checklist

Run at the beginning:

cd ~/Desktop
mkdir exam_project
cd exam_project

git clone https://github.com/oritidhar/OS_Project_Graphs.git
cd OS_Project_Graphs

git checkout main
git pull origin main

git checkout -b exam_a/ID_NUMBER

code .

Build and run according to the relevant milestone.

Example for Milestone 4:

make milestone4

./sim tests/milestone4/test_m4.txt

Run at the end:

git status

git diff

git add .

git commit -m "exam_a: stages 1,2,3 - implemented task"

git push -u origin exam_a/ID_NUMBER

# 18. Main rule

Do not refactor.
Do not change unrelated files.
Do not work on main.
Do not try to improve the whole project.

Do:
- Find the correct location
- Make a small focused change
- Write logical code
- Build if possible
- Commit
- Push