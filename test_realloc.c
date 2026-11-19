#include <stdio.h>
#include "myalloc.h"

int main(void)
{
    /* Allocate memory for two integers. */
    int *numbers = my_malloc(2 * sizeof(int));

    if (numbers == NULL) {
        printf("Initial allocation failed\n");
        return 1;
    }

    numbers[0] = 100;
    numbers[1] = 200;

    /* Allocate another block to prevent adjacent expansion. */
    void *blocker = my_malloc(100);

    if (blocker == NULL) {
        printf("Blocker allocation failed\n");
        my_free(numbers);
        return 1;
    }

    /* Increase the original allocation to hold ten integers. */
    int *resized = my_realloc(numbers, 10 * sizeof(int));

    if (resized == NULL) {
        printf("Reallocation failed\n");
        my_free(numbers);
        my_free(blocker);
        return 1;
    }

    /* Verify that the original contents were preserved. */
    printf("Realloc preserved contents: %s\n",
           (resized[0] == 100 && resized[1] == 200)
           ? "PASS" : "FAIL");

    /* Check that the new allocation has a different address. */
    printf("Moved to a new block: %s\n",
           (resized != numbers) ? "YES" : "NO");

    /* Test shrinking the allocation. */
    int *smaller = my_realloc(resized, sizeof(int));

    if (smaller == NULL) {
        printf("Shrinking failed\n");
        my_free(resized);
        my_free(blocker);
        return 1;
    }

    printf("Realloc shrinking preserved data: %s\n",
           smaller[0] == 100 ? "PASS" : "FAIL");

    my_free(smaller);
    my_free(blocker);

    printf("Memory released successfully.\n");

    return 0;
}
