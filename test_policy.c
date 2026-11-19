#include <stdio.h>
#include "myalloc.h"

int main(void)
{
    void *first = my_malloc(100);
    void *second = my_malloc(200);
    void *third = my_malloc(300);

    if (first == NULL || second == NULL || third == NULL) {
        printf("Allocation failed\n");
        return 1;
    }

    my_free(first);
    my_free(third);

    set_policy(FIT_BEST);

    void *result = my_malloc(80);

    if (result == first) {
        printf("Best-fit selected the smaller suitable block: PASS\n");
    } else {
        printf("Best-fit test: FAIL\n");
    }

    my_free(result);
    my_free(second);

    return 0;
}
