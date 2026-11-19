#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "myalloc.h"

int main(void)
{
    char *ptr = my_malloc(100);

    if (ptr == NULL) {
        printf("Allocation failed\n");
        return 1;
    }

    printf("Allocated address: %p\n", (void *)ptr);

    printf("16-byte aligned: %s\n",
           ((uintptr_t)ptr % 16 == 0) ? "YES" : "NO");

    memset(ptr, 'A', 100);

    printf("First byte: %c\n", ptr[0]);
    printf("Last byte: %c\n", ptr[99]);

    return 0;
}
