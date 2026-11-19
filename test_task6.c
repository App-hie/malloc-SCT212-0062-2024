#include <stdio.h>
#include <string.h>
#include "myalloc.h"

int main(void)
{
    /* Test calloc. */
    int *numbers = my_calloc(5, sizeof(int));

    if (numbers == NULL) {
        printf("Calloc failed\n");
        return 1;
    }

    printf("Calloc zero initialization: ");

    int all_zero = 1;

    for (int i = 0; i < 5; i++) {
        if (numbers[i] != 0) {
            all_zero = 0;
        }
    }

    printf("%s\n", all_zero ? "PASS" : "FAIL");

    /* Store values in the allocated memory. */
    numbers[0] = 10;
    numbers[1] = 20;
    numbers[2] = 30;

    /* Test realloc by increasing the allocation size. */
    int *resized = my_realloc(numbers, 10 * sizeof(int));

    if (resized == NULL) {
        printf("Realloc failed\n");
        my_free(numbers);
        return 1;
    }

    printf("Realloc preserved contents: %s\n",
           (resized[0] == 10 &&
            resized[1] == 20 &&
            resized[2] == 30) ? "PASS" : "FAIL");

    /* Test shrinking the allocation. */
    int *smaller = my_realloc(resized, 2 * sizeof(int));

    if (smaller == NULL) {
        printf("Realloc shrinking failed\n");
        my_free(resized);
        return 1;
    }

    printf("Realloc shrinking preserved contents: %s\n",
           (smaller[0] == 10 && smaller[1] == 20) ? "PASS" : "FAIL");

    my_free(smaller);

    printf("Memory released successfully.\n");

    return 0;
}
