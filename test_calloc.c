#include <stdio.h>
#include "myalloc.h"

int main(void)
{
    int *numbers = my_calloc(5, sizeof(int));

    if (numbers == NULL) {
        printf("Allocation failed\n");
        return 1;
    }

    int all_zero = 1;

    for (int i = 0; i < 5; i++) {
        if (numbers[i] != 0) {
            all_zero = 0;
        }
    }

    printf("Calloc zero initialization: %s\n",
           all_zero ? "PASS" : "FAIL");

    my_free(numbers);

    printf("Memory released successfully.\n");

    return 0;
}
