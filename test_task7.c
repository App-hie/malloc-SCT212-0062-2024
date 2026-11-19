
#include <stdio.h>
#include "myalloc.h"

int main(void)
{
    printf("Initial heap check: %s\n",
           heap_check() ? "PASS" : "FAIL");

    void *first = my_malloc(100);
    void *second = my_malloc(200);
    void *third = my_malloc(300);

    if (first == NULL || second == NULL || third == NULL) {
        printf("Allocation failed\n");
        return 1;
    }

    my_free(second);

    printf("Heap check after freeing a block: %s\n",
           heap_check() ? "PASS" : "FAIL");

    heap_stats_t stats;
    heap_stats(&stats);

    printf("\nHeap Statistics:\n");
    printf("Heap bytes: %zu\n", stats.heap_bytes);
    printf("Used bytes: %zu\n", stats.used_bytes);
    printf("Free bytes: %zu\n", stats.free_bytes);
    printf("Total blocks: %zu\n", stats.blocks);
    printf("Free blocks: %zu\n", stats.free_blocks);
    printf("Largest free block: %zu\n", stats.largest_free);

    printf("\nHeap Dump:\n");
    heap_dump();

    my_free(first);
    my_free(third);

    printf("\nHeap check after freeing all allocations: %s\n",
           heap_check() ? "PASS" : "FAIL");

    return 0;
}
