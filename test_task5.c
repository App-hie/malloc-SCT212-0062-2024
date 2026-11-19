#include <stdio.h>
#include "myalloc.h"

int main(void)
{
    char *first = my_malloc(1000);
    char *second = my_malloc(1000);

    if (first == NULL || second == NULL) {
        printf("Allocation failed\n");
        return 1;
    }

    printf("First block:  %p\n", (void *)first);
    printf("Second block: %p\n", (void *)second);

    /* Free both blocks. */
    my_free(first);
    my_free(second);

    printf("Both blocks freed successfully.\n");

    /* Test whether the merged memory can be reused. */
    char *third = my_malloc(1500);

    if (third == NULL) {
        printf("Reallocation failed\n");
        return 1;
    }

    printf("Third allocation: %p\n", (void *)third);

    printf("Coalescing successful: %s\n",
           (third == first) ? "YES" : "NO");

    return 0;
}
