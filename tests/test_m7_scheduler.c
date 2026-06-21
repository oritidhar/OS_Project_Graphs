/*
 * Headless M7 scheduler test.
 *
 * Verifies the core milestone-7 requirement: FCFS and SJF dequeue waiting
 * travelers in different, well-defined orders for the SAME enqueue sequence.
 *
 * Scenario mirrors demo_schedulers.txt — three travelers reach the same node
 * in input order long(7) → medium(4) → short(2):
 *   FCFS must return them in arrival order   : long, medium, short
 *   SJF  must return shortest-remaining first : short, medium, long
 *
 * Links against the project's scheduler objects; no raylib symbols are used.
 */

#include <stdio.h>
#include "core/scheduler.h"

static TravelerInfo mk(pid_t pid) {
    TravelerInfo t = (TravelerInfo){0};
    t.pid = pid;
    return t;
}

/* Enqueue long(pid100,rem7), medium(pid101,rem4), short(pid102,rem2) at node 3
 * and return the three dequeued PIDs in order. */
static void run_order(SchedulerType type, pid_t out[3]) {
    scheduler_init(type);
    scheduler_enqueue_with_remaining(3, mk(100), 7);  /* long   */
    scheduler_enqueue_with_remaining(3, mk(101), 4);  /* medium */
    scheduler_enqueue_with_remaining(3, mk(102), 2);  /* short  */
    out[0] = scheduler_next(3);
    out[1] = scheduler_next(3);
    out[2] = scheduler_next(3);
}

static int check(const char* name, pid_t got[3], pid_t a, pid_t b, pid_t c) {
    int ok = (got[0] == a && got[1] == b && got[2] == c);
    printf("  %-5s order: %ld, %ld, %ld  [%s]\n", name,
           (long)got[0], (long)got[1], (long)got[2], ok ? "OK" : "FAIL");
    return ok;
}

int main(void) {
    int pass = 0, total = 0;
    pid_t fcfs[3], sjf[3];

    run_order(FCFS, fcfs);
    run_order(SJF,  sjf);

    printf("TEST : FCFS keeps arrival order, SJF prefers fewest hops\n");
    total++; pass += check("FCFS", fcfs, 100, 101, 102);  /* arrival order   */
    total++; pass += check("SJF",  sjf,  102, 101, 100);  /* shortest first  */

    /* The two algorithms must actually differ on this input. */
    total++;
    int differ = (fcfs[0] != sjf[0]);
    printf("  schedulers differ on same input: %s\n", differ ? "OK" : "FAIL");
    pass += differ;

    /* Empty-queue contract: scheduler_next() returns -1. */
    total++;
    scheduler_init(FCFS);
    int empty_ok = (scheduler_next(0) == -1);
    printf("  empty queue returns -1: %s\n", empty_ok ? "OK" : "FAIL");
    pass += empty_ok;

    printf("\nSUMMARY: %d / %d passed\n", pass, total);
    return (pass == total) ? 0 : 1;
}
