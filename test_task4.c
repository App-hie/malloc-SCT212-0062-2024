#include <stdio.h>
#include <stdint.h>
#include "myalloc.h"

int main(void)
{
    char *first = my_malloc(100);
    char *second = my_malloc(200);

    if (first == NULL || second == NULL) {
        printf("Allocation failed\n");
        return 1;
    }

    printf("First allocation:  %p\n", (void *)first);
    printf("Second allocation: %p\n", (void *)second);

    printf("First aligned:  %s\n",
           ((uintptr_t)first % 16 == 0) ? "YES" : "NO");

    printf("Second aligned: %s\n",
           ((uintptr_t)second % 16 == 0) ? "YES" : "NO");

    printf("Different addresses: %s\n",
           (first != second) ? "YES" : "NO");

    return 0;
}
