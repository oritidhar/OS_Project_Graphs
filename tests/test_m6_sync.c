/*
 * Headless M6 synchronization test harness.
 * No raylib dependency — links only graph/dijkstra/ipc/sync.
 *
 * Mutual exclusion is verified via shared memory (mmap MAP_SHARED):
 *   children atomically increment occ_count[node] after acquiring the
 *   semaphore-lock and decrement before releasing it.  The parent samples
 *   the array continuously; any count > 1 is a real violation.
 *
 * Message-based IPC is still used to detect starvation (all travelers
 * finish) and to count waiting/entered-after-wait events.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>

#include "core/graph.h"
#include "core/dijkstra.h"
#include "core/ipc.h"
#include "core/sync.h"

#define MAX_T   32
#define MAX_N   256
#define LINE_SZ 256

/* ── shared memory: real-time per-node occupancy count ────────────── */

static volatile int* occ_count = NULL;   /* set in parent before fork */

/* ── minimal file parser ─────────────────────────────────────────── */

typedef struct { int src; int dst; } TSpec;

static int skip_comment(FILE* f, char* buf, int sz) {
    while (fgets(buf, sz, f)) {
        char* p = buf;
        while (*p == ' ' || *p == '\t') p++;
        if (*p != '#' && *p != '\n' && *p != '\r' && *p != '\0') return 1;
    }
    return 0;
}

static Graph* parse_file(const char* path, TSpec* specs, int* count) {
    *count = 0;
    FILE* f = fopen(path, "r");
    if (!f) { perror(path); return NULL; }

    char line[LINE_SZ];
    int n = 0, m = 0;
    if (!skip_comment(f, line, sizeof line) ||
        sscanf(line, "%d %d", &n, &m) != 2 || n <= 0 || m < 0) {
        fprintf(stderr, "bad header in %s\n", path);
        fclose(f); return NULL;
    }

    Graph* g = createGraph(n);
    for (int i = 0; i < m; i++) {
        if (!skip_comment(f, line, sizeof line)) {
            fprintf(stderr, "truncated edges in %s\n", path);
            freeGraph(g); fclose(f); return NULL;
        }
        int u, v, w;
        if (sscanf(line, "%d %d %d", &u, &v, &w) != 3) {
            fprintf(stderr, "bad edge line: %s\n", line);
            freeGraph(g); fclose(f); return NULL;
        }
        addEdge(g, u, v, w);
    }

    int nt = 0;
    if (!skip_comment(f, line, sizeof line) || sscanf(line, "%d", &nt) != 1) {
        fprintf(stderr, "missing traveler count in %s\n", path);
        freeGraph(g); fclose(f); return NULL;
    }
    for (int i = 0; i < nt && *count < MAX_T; i++) {
        if (!skip_comment(f, line, sizeof line)) break;
        int s, d;
        if (sscanf(line, "%d %d", &s, &d) == 2)
            specs[(*count)++] = (TSpec){s, d};
    }
    fclose(f);
    return g;
}

/* ── child process: IPC + sync + shared-memory occupancy tracking ─── */

static void child_run(int write_fd, Graph* graph, int src, int dst) {
    PathResult* result = dijkstra_compute_path(graph, src, dst);
    if (!result) {
        fprintf(stderr, "[CHILD %d] no path %d->%d\n", getpid(), src, dst);
        close(write_fd);
        exit(EXIT_FAILURE);
    }

    for (int step = 0; step < result->path_len; step++) {
        int node = result->path[step];
        int next = (step + 1 < result->path_len) ? result->path[step + 1] : -1;

        if (!node_try_lock(node)) {
            IPCMessage wm = { getpid(), node, next, false, true, node };
            ipc_send(write_fd, &wm);
            node_lock(node);
        }

        /* Under exclusive lock — increment shared occupancy counter */
        __atomic_fetch_add((int*)&occ_count[node], 1, __ATOMIC_SEQ_CST);

        IPCMessage msg = { getpid(), node, next, false, false, -1 };
        ipc_send(write_fd, &msg);

        sleep(1);

        /* Decrement before releasing so count never dips below 0 first */
        __atomic_fetch_sub((int*)&occ_count[node], 1, __ATOMIC_SEQ_CST);
        node_unlock(node);
        if (next != -1) usleep(400000);
    }

    IPCMessage done = { getpid(), dst, -1, true, false, -1 };
    ipc_send(write_fd, &done);
    free_path_result(result);
    close(write_fd);
    exit(EXIT_SUCCESS);
}

/* ── timeout via SIGALRM ─────────────────────────────────────────── */

static volatile sig_atomic_t timed_out = 0;
static void on_alarm(int sig) { (void)sig; timed_out = 1; }

/* ── single test runner ──────────────────────────────────────────── */

static bool run_test(const char* label, const char* filepath, int timeout_secs) {
    printf("\n──────────────────────────────────────────\n");
    printf("TEST : %s\n", label);
    printf("FILE : %s\n", filepath);

    TSpec specs[MAX_T];
    int count = 0;
    Graph* graph = parse_file(filepath, specs, &count);
    if (!graph || count == 0) {
        printf("RESULT: FAIL (parse error)\n");
        return false;
    }
    printf("  graph: %d nodes | travelers: %d\n", graph->numVertices, count);

    /* Shared occupancy counters — visible to all children via MAP_SHARED */
    occ_count = mmap(NULL, MAX_N * sizeof(int),
                     PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
                     -1, 0);
    if (occ_count == MAP_FAILED) {
        perror("mmap"); freeGraph(graph); return false;
    }
    memset((void*)occ_count, 0, MAX_N * sizeof(int));

    int pipe_fds[MAX_T][2];
    for (int i = 0; i < MAX_T; i++) pipe_fds[i][0] = pipe_fds[i][1] = -1;

    if (ipc_open_pipes(pipe_fds, count) != 0) {
        printf("RESULT: FAIL (pipe setup)\n");
        munmap((void*)occ_count, MAX_N * sizeof(int));
        freeGraph(graph); return false;
    }

    if (sync_init(graph->numVertices) != 0) {
        printf("RESULT: FAIL (sync_init)\n");
        munmap((void*)occ_count, MAX_N * sizeof(int));
        freeGraph(graph); return false;
    }

    pid_t pids[MAX_T];
    for (int i = 0; i < count; i++) {
        pids[i] = fork();
        if (pids[i] < 0) { perror("fork"); exit(1); }
        if (pids[i] == 0) {
            for (int j = 0; j < count; j++) close(pipe_fds[j][0]);
            for (int j = 0; j < count; j++) if (j != i) close(pipe_fds[j][1]);
            child_run(pipe_fds[i][1], graph, specs[i].src, specs[i].dst);
        }
    }
    for (int i = 0; i < count; i++) close(pipe_fds[i][1]);
    sync_cleanup();  /* parent frees handles; children keep their inherited refs */

    /* ── parent monitoring ── */
    int  max_occ[MAX_N];      /* highest occupancy seen per node */
    bool alive[MAX_T];
    int  waiting_msgs = 0;
    int  entered_waited = 0;
    int  finished = 0;
    bool mutual_ex_ok = true;
    int  violations = 0;
    bool waited_for[MAX_T][MAX_N];

    memset(max_occ,    0, sizeof max_occ);
    memset(waited_for, 0, sizeof waited_for);
    for (int i = 0; i < count; i++) alive[i] = true;

    timed_out = 0;
    signal(SIGALRM, on_alarm);
    alarm((unsigned)timeout_secs);

    while (finished < count && !timed_out) {
        /* Sample shared occupancy counters */
        for (int n = 0; n < graph->numVertices; n++) {
            int c = occ_count[n];
            if (c > max_occ[n]) max_occ[n] = c;
            if (c > 1) {
                printf("  *** VIOLATION: node %d has occupancy %d ***\n", n, c);
                mutual_ex_ok = false;
                violations++;
            }
        }

        /* Read IPC messages for starvation / log checks */
        for (int i = 0; i < count; i++) {
            if (!alive[i] || pipe_fds[i][0] < 0) continue;
            IPCMessage msg;
            int rc = ipc_recv(pipe_fds[i][0], &msg);
            if (rc == 0) continue;
            if (rc == -1) {
                printf("  [T%d] pipe closed early\n", i + 1);
                alive[i] = false; finished++;
                close(pipe_fds[i][0]); pipe_fds[i][0] = -1;
                continue;
            }

            if (msg.waiting_for_node) {
                int bn = msg.blocked_at_node;
                if (bn >= 0 && bn < MAX_N) waited_for[i][bn] = true;
                waiting_msgs++;
                printf("  [T%d] waiting for node %d\n", i + 1, bn);
                continue;
            }

            if (msg.finished) {
                printf("  [T%d] finished\n", i + 1);
                alive[i] = false; finished++;
                close(pipe_fds[i][0]); pipe_fds[i][0] = -1;
                continue;
            }

            int node = msg.current_node;
            bool w = (node >= 0 && node < MAX_N && waited_for[i][node]);
            if (w) { entered_waited++; memset(waited_for[i], 0, MAX_N); }
            printf("  [T%d] entered node %d%s -> next %d\n",
                   i + 1, node, w ? " (was waiting)" : "", msg.next_node);
        }

        usleep(2000);   /* 2 ms poll interval */
    }

    alarm(0);

    /* Drain remaining pipes and reap children */
    for (int i = 0; i < count; i++) {
        if (pipe_fds[i][0] >= 0) close(pipe_fds[i][0]);
        waitpid(pids[i], NULL, 0);
    }

    /* Print per-node max occupancy only for nodes that were actually used */
    printf("  Max occupancy per node:\n");
    for (int n = 0; n < graph->numVertices; n++)
        if (max_occ[n] > 0)
            printf("    node %2d: max=%d  %s\n", n, max_occ[n],
                   max_occ[n] > 1 ? "<-- VIOLATION" : "");

    bool all_done = (finished == count);
    printf("\n  Mutual exclusion : %s  (%d real-time violation(s))\n",
           mutual_ex_ok ? "OK" : "VIOLATED", violations);
    printf("  No starvation    : %s  (%d/%d finished)\n",
           all_done ? "OK" : "FAIL", finished, count);
    printf("  Waiting messages : %d  |  entered-after-wait: %d\n",
           waiting_msgs, entered_waited);
    if (timed_out) printf("  *** TIMEOUT after %d s ***\n", timeout_secs);

    bool pass = mutual_ex_ok && all_done && !timed_out;
    printf("RESULT: %s\n", pass ? "PASS" : "FAIL");

    munmap((void*)occ_count, MAX_N * sizeof(int));
    occ_count = NULL;
    freeGraph(graph);
    return pass;
}

/* ── main ────────────────────────────────────────────────────────── */

int main(void) {
    int pass = 0, total = 0;

#define T(label, file, secs) \
    do { total++; if (run_test(label, file, secs)) pass++; } while (0)

    T("Single traveler (baseline, no contention)",
      "assets/samples/test_m6_single.txt",          10);

    T("Two travelers, identical path (maximum contention)",
      "assets/samples/test_m6_two_conflict.txt",    25);

    T("src == dst edge case (path_len=1)",
      "assets/samples/test_m6_src_eq_dst.txt",      15);

    T("4 travelers, all start at same node",
      "assets/samples/test_m6_same_start.txt",      45);

    T("4 travelers, chain stagger (converging paths)",
      "assets/samples/test_m6_chain.txt",           45);

    T("6 travelers, double bottleneck at nodes 5 and 7",
      "assets/samples/test_m6_complex.txt",        100);

#undef T

    printf("\n══════════════════════════════════════════\n");
    printf("SUMMARY: %d / %d passed\n", pass, total);
    printf("══════════════════════════════════════════\n");
    return (pass == total) ? 0 : 1;
}
