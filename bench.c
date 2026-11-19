
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "myalloc.h"

#define OPERATIONS 50000
#define SLOTS 1000
#define MAX_SIZE 4096
#define RANDOM_SEED 42

/* Calculate elapsed time in seconds. */
static double elapsed_time(struct timespec start,
                           struct timespec end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_nsec - start.tv_nsec) / 1000000000.0;
}

/* Run the benchmark using our custom allocator. */
static void benchmark_custom(fit_policy_t policy,
                             const char *policy_name)
{
    void *slots[SLOTS] = {0};

    set_policy(policy);
    srand(RANDOM_SEED);

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < OPERATIONS; i++) {
        int index = rand() % SLOTS;

        if (slots[index] != NULL) {
            my_free(slots[index]);
            slots[index] = NULL;
        } else {
            size_t size = (size_t)(rand() % MAX_SIZE) + 1;
            slots[index] = my_malloc(size);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    heap_stats_t stats;
    heap_stats(&stats);

    double time_taken = elapsed_time(start, end);

    double fragmentation = 0.0;

    if (stats.free_bytes > 0) {
        fragmentation =
            1.0 - ((double)stats.largest_free /
                   (double)stats.free_bytes);
    }

    printf("\n--- %s ---\n", policy_name);
    printf("Operations: %d\n", OPERATIONS);
    printf("Execution time: %.6f seconds\n", time_taken);
    printf("Heap bytes: %zu\n", stats.heap_bytes);
    printf("Used bytes: %zu\n", stats.used_bytes);
    printf("Free bytes: %zu\n", stats.free_bytes);
    printf("Free blocks: %zu\n", stats.free_blocks);
    printf("Largest free block: %zu\n", stats.largest_free);
    printf("External fragmentation: %.4f (%.2f%%)\n",
           fragmentation, fragmentation * 100.0);
    printf("Heap consistency: %s\n",
           heap_check() ? "PASS" : "FAIL");

    /* Release remaining allocations. */
    for (int i = 0; i < SLOTS; i++) {
        if (slots[i] != NULL) {
            my_free(slots[i]);
        }
    }
}

/* Run the benchmark using standard C malloc/free. */
static void benchmark_libc(void)
{
    void *slots[SLOTS] = {0};

    srand(RANDOM_SEED);

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < OPERATIONS; i++) {
        int index = rand() % SLOTS;

        if (slots[index] != NULL) {
            free(slots[index]);
            slots[index] = NULL;
        } else {
            size_t size = (size_t)(rand() % MAX_SIZE) + 1;
            slots[index] = malloc(size);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("\n--- Standard C malloc/free ---\n");
    printf("Operations: %d\n", OPERATIONS);
    printf("Execution time: %.6f seconds\n",
           elapsed_time(start, end));
    printf("Heap statistics: Not measured by this benchmark\n");

    for (int i = 0; i < SLOTS; i++) {
        free(slots[i]);
    }
}

int main(void)
{
    printf("MEMORY ALLOCATOR BENCHMARK\n");
    printf("Operations: %d\n", OPERATIONS);
    printf("Allocation sizes: 1 - %d bytes\n", MAX_SIZE);
    printf("Random seed: %d\n", RANDOM_SEED);

    benchmark_custom(FIT_FIRST, "First-fit");
    benchmark_custom(FIT_BEST, "Best-fit");
    benchmark_libc();

    return 0;
}
