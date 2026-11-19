#define _DEFAULT_SOURCE
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include "myalloc.h"
#include <stdio.h>

/* Identifies a valid memory block. */
#define BLOCK_MAGIC UINT32_C(0xCAFEBABE)

/* Memory block structure. */
typedef struct block {
    size_t size;
    struct block *next;
    int free;
    uint32_t magic;
} block_t;

/* Round a number up to the nearest multiple of 16. */
static size_t align_up(size_t n)
{
    return (n + 15) & ~(size_t)15;
}

/* Calculate the address of the payload. */
static void *payload(block_t *block)
{
    return (void *)((char *)block + align_up(sizeof(block_t)));
}

/* Calculate the address of the header from a payload pointer. */
static block_t *header(void *ptr)
{
    return (block_t *)((char *)ptr - align_up(sizeof(block_t)));
}

/* Header size rounded up to a multiple of 16. */
#define HDR align_up(sizeof(block_t))

/* Global pointers for tracking the linked list. */
static block_t *head = NULL;
static block_t *tail = NULL;

/* Total memory obtained from the operating system. */
static size_t heap_bytes = 0;

/* Current allocation policy. First-fit is the default. */
static fit_policy_t current_policy = FIT_FIRST;

/* Change the allocator's placement policy. */
void set_policy(fit_policy_t p)
{
    if (p == FIT_FIRST || p == FIT_BEST) {
        current_policy = p;
    }
}
/* Obtain additional memory from the operating system. */
static block_t *grow(size_t need)
{
    const size_t CHUNK_SIZE = 64 * 1024;

    /* Check the current program break. */
    void *current_break = sbrk(0);

    if (current_break == (void *)-1) {
        return NULL;
    }

    /* Calculate padding required for 16-byte alignment. */
    uintptr_t address = (uintptr_t)current_break;
    size_t padding = (16 - (address % 16)) % 16;

    /* Request at least 64 KB of payload space. */
    size_t capacity = need;

    if (capacity < CHUNK_SIZE) {
        capacity = CHUNK_SIZE;
    }

    /* Protect against size and sbrk increment overflow. */
    if (capacity > (size_t)INTPTR_MAX - HDR - padding) {
        return NULL;
    }

    size_t total = padding + HDR + capacity;

    /* Request memory by moving the program break. */
    void *memory = sbrk((intptr_t)total);

    if (memory == (void *)-1) {
        return NULL;
    }

    /* Align the beginning of the new block. */
    uintptr_t aligned_address =
        ((uintptr_t)memory + 15) & ~(uintptr_t)15;

    block_t *new_block = (block_t *)aligned_address;

    /* Initialize the new block. */
    new_block->size = capacity;
    new_block->next = NULL;
    new_block->free = 1;
    new_block->magic = BLOCK_MAGIC;

    /* Append the block to the linked list. */
    if (head == NULL) {
        head = new_block;
        tail = new_block;
    } else {
        tail->next = new_block;
        tail = new_block;
    }

    /* Update total memory obtained from the OS. */
    heap_bytes += total;

    return new_block;
}
/* Find a suitable free block using the selected placement policy. */
static block_t *find_fit(size_t need)
{
    block_t *current = head;
    block_t *best = NULL;

    while (current != NULL) {

        if (current->free && current->size >= need) {

            /* First-fit: return the first suitable block. */
            if (current_policy == FIT_FIRST) {
                return current;
            }

            /* Best-fit: remember the smallest suitable block. */
            if (best == NULL || current->size < best->size) {
                best = current;
            }
        }

        current = current->next;
    }

    return best;
}

/* Split a large free block when enough space remains. */
static void split_block(block_t *block, size_t need)
{
    /* Check whether the remainder can hold a header and 16-byte payload. */
    if (block->size - need < HDR + 16) {
        return;
    }

    /* Calculate where the new free block should begin. */
    block_t *new_block =
        (block_t *)((char *)block + HDR + need);

    /* Initialize the new free block. */
    new_block->size = block->size - need - HDR;
    new_block->next = block->next;
    new_block->free = 1;
    new_block->magic = BLOCK_MAGIC;

    /* Update the original block. */
    block->size = need;
    block->next = new_block;

    /* Update the tail if the original block was the last block. */
    if (tail == block) {
        tail = new_block;
    }
}
/* Check whether two blocks are physically next to each other. */
static int are_adjacent(block_t *first, block_t *second)
{
    char *end_of_first =
        (char *)first + HDR + first->size;

    return end_of_first == (char *)second;
}
/* Release an allocated block and merge adjacent free blocks. */
void my_free(void *ptr)
{
    /* Ignore NULL pointers. */
    if (ptr == NULL) {
        return;
    }

    block_t *current = head;
    block_t *previous = NULL;

    /* Find the block whose payload matches the supplied pointer. */
    while (current != NULL && payload(current) != ptr) {
        previous = current;
        current = current->next;
    }

    /* Reject pointers that do not belong to this allocator. */
    if (current == NULL) {
        const char message[] = "my_free: invalid pointer\n";
        write(STDERR_FILENO, message, sizeof(message) - 1);
        return;
    }

    /* Use the header helper after confirming the pointer is valid. */
    block_t *block = header(ptr);

    /* Check the block's magic value. */
    if (block->magic != BLOCK_MAGIC) {
        const char message[] = "my_free: invalid block\n";
        write(STDERR_FILENO, message, sizeof(message) - 1);
        return;
    }

    /* Detect attempts to free an already-free block. */
    if (block->free) {
        const char message[] = "my_free: double free detected\n";
        write(STDERR_FILENO, message, sizeof(message) - 1);
        return;
    }

    /* Mark the block as free. */
    block->free = 1;

    /*
     * Merge with the previous block if it is free
     * and physically adjacent.
     */
    if (previous != NULL &&
        previous->free &&
        are_adjacent(previous, block)) {

        previous->size += HDR + block->size;
        previous->next = block->next;

        if (tail == block) {
            tail = previous;
        }

        block = previous;
    }

    /*
     * Merge with subsequent free blocks whenever
     * they are physically adjacent.
     */
    while (block->next != NULL &&
           block->next->free &&
           are_adjacent(block, block->next)) {

        block_t *next_block = block->next;

        block->size += HDR + next_block->size;
        block->next = next_block->next;

        if (tail == next_block) {
            tail = block;
        }
    }
}
/* Allocate memory using the first-fit policy. */
void *my_malloc(size_t size)
{
    /* A zero-byte request returns NULL. */
    if (size == 0) {
        return NULL;
    }

    /* Prevent overflow when aligning the requested size. */
    if (size > SIZE_MAX - 15) {
        return NULL;
    }

    /* Round the requested size up to 16-byte alignment. */
    size_t need = align_up(size);

    /* Search for an existing free block. */
    block_t *block = find_fit(need);

    /* Obtain more memory if no suitable block exists. */
    if (block == NULL) {
        block = grow(need);
    }

    if (block == NULL) {
        return NULL;
    }
    /* Split the block if it is larger than necessary. */
    split_block(block, need);

    /* Mark the block as allocated. */
    block->free = 0;

    /* Return the payload, not the block header. */
    return payload(block);

}

/* Allocate memory for multiple elements and initialize it to zero. */
void *my_calloc(size_t nmemb, size_t size)
{
    /* Prevent multiplication overflow. */
    if (nmemb != 0 && size > SIZE_MAX / nmemb) {
        return NULL;
    }

    /* Calculate the total number of bytes required. */
    size_t total = nmemb * size;

    /* Allocate the required memory. */
    void *ptr = my_malloc(total);

    if (ptr == NULL) {
        return NULL;
    }

    /* Initialize every allocated byte to zero. */
    memset(ptr, 0, total);

    return ptr;
}
/* Resize an existing allocation while preserving its contents. */
void *my_realloc(void *ptr, size_t size)
{
    /* If the original pointer is NULL, behave like my_malloc(). */
    if (ptr == NULL) {
        return my_malloc(size);
    }

    /* A zero-size request frees the original allocation. */
    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    /* Protect against alignment overflow. */
    if (size > SIZE_MAX - 15) {
        return NULL;
    }

    /* Find the existing block in the allocator's linked list. */
    block_t *block = head;

    while (block != NULL && payload(block) != ptr) {
        block = block->next;
    }

    /* Reject pointers that do not belong to this allocator. */
    if (block == NULL || block->magic != BLOCK_MAGIC || block->free) {
        return NULL;
    }

    size_t need = align_up(size);

    /* CASE 1:existing block is already large enough. */
    if (need <= block->size) {

        /* Split the block if it has excess space. */
        split_block(block, need);

        /* Merge the new free remainder with following free blocks. */
        while (block->next != NULL &&
               block->next->free &&
               are_adjacent(block, block->next)) {

            block_t *next_block = block->next;

            block->size += HDR + next_block->size;
            block->next = next_block->next;

            if (tail == next_block) {
                tail = block;
            }
        }

        return ptr;
    }

    /* CASE 2:expanding into the free block(s). */
    while (block->next != NULL &&
           block->next->free &&
           are_adjacent(block, block->next) &&
           block->size < need) {

        block_t *next_block = block->next;

        /* Prevent size arithmetic overflow. */
        if (block->size > SIZE_MAX - HDR - next_block->size) {
            return NULL;
        }

        block->size += HDR + next_block->size;
        block->next = next_block->next;

        if (tail == next_block) {
            tail = block;
        }
    }

    /* If the combined block is now large enough, use it. */
    if (block->size >= need) {
        split_block(block, need);
        return ptr;
    }

    /* CASE 3: Allocate a new block if expansion was unsuccessful. */
    void *new_ptr = my_malloc(size);

    if (new_ptr == NULL) {
        return NULL;
    }

    /* Copy the contents from the old block to the new block. */
    memcpy(new_ptr, ptr, block->size);

    /* Release the old allocation. */
    my_free(ptr);

    return new_ptr;
}
void heap_stats(heap_stats_t *out)
{
    if (out == NULL) {
        return;
    }

    out->heap_bytes = heap_bytes;
    out->used_bytes = 0;
    out->free_bytes = 0;
    out->blocks = 0;
    out->free_blocks = 0;
    out->largest_free = 0;

    block_t *current = head;

    while (current != NULL) {
        out->blocks++;

        if (current->free) {
            out->free_blocks++;
            out->free_bytes += current->size;

            if (current->size > out->largest_free) {
                out->largest_free = current->size;
            }
        } else {
            out->used_bytes += current->size;
        }

        current = current->next;
    }
}
void heap_dump(void)
{
    block_t *current = head;
    size_t index = 0;
    char buffer[160];

    while (current != NULL) {
        int length = snprintf(
            buffer,
            sizeof(buffer),
            "Block %zu: address=%p, size=%zu, status=%s, magic=%08x\n",
            index,
            (void *)current,
            current->size,
            current->free ? "FREE" : "USED",
            (unsigned int)current->magic
        );

        if (length > 0) {
            write(STDOUT_FILENO, buffer, (size_t)length);
        }

        current = current->next;
        index++;
    }
}

int heap_check(void)
{
    /* An empty heap must have both pointers set to NULL. */
    if (head == NULL) {
        return tail == NULL;
    }

    if (tail == NULL) {
        return 0;
    }

    block_t *current = head;
    block_t *previous = NULL;

    while (current != NULL) {

        /* Check the block's magic number. */
        if (current->magic != BLOCK_MAGIC) {
            return 0;
        }

        /* Check 16-byte alignment of the header and payload. */
        if ((uintptr_t)current % 16 != 0) {
            return 0;
        }

        if ((uintptr_t)payload(current) % 16 != 0) {
            return 0;
        }

        /* Every block's payload size must be 16-byte aligned. */
        if (current->size % 16 != 0) {
            return 0;
        }

        /* Blocks must appear in increasing address order. */
        if (previous != NULL) {
            if ((uintptr_t)current <= (uintptr_t)previous) {
                return 0;
            }

            /* Adjacent free blocks should already be merged. */
            if (previous->free && current->free &&
                are_adjacent(previous, current)) {
                return 0;
            }
        }

        previous = current;
        current = current->next;
    }

    /* The tail must be the final block in the list. */
    if (previous != tail || tail->next != NULL) {
        return 0;
    }

    return 1;
}
