# Custom Memory Allocator in C

## Student Information

| Detail | Information |
|---|---|
| Student Name | INDAKWA PAULINE APPHIE |
| Registration Number | SCT212-0062/2024 |
| Unit | Systems Programming |
| Custom Memory Allocator |
| Operating Environment | Ubuntu on Windows Subsystem for Linux (WSL) |

## 1. Project Overview

This project implements a custom dynamic memory allocator in C using the `sbrk()` system call to obtain memory from the operating system.

The allocator provides functionality similar to the standard C memory-management functions through the following custom functions:

- `my_malloc()` – allocates memory.
- `my_free()` – releases allocated memory.
- `my_calloc()` – allocates memory and initializes it to zero.
- `my_realloc()` – resizes previously allocated memory.

It also supports first-fit and best-fit allocation policies, block splitting, coalescing of adjacent free blocks, heap statistics, heap inspection and consistency checking.

## 2. Building and Running the Project

### Requirements

- GCC compiler.
- Ubuntu/Linux environment or WSL.
- GNU Make.
- C compiler supporting C11.

### Compile and run the supplied test suite

Navigate to the project directory:

```bash
cd ~/malloc_assignment
```

Compile and execute the tests:

```bash
make
```

Clean generated files:

```bash
make clean
```

### Compile and run the benchmark

Compile:

```bash
gcc -std=c11 -Wall -Wextra -g -O0 -o bench myalloc.c bench.c
```

Execute:

```bash
./bench
```

The benchmark compares first-fit, best-fit and the standard C library's `malloc()` and `free()` functions.

## 3. Project Files

| File | Description |
|---|---|
| `myalloc.c` | Main implementation of the custom memory allocator. |
| `myalloc.h` | Header containing the required API, allocation policies and heap statistics structure. |
| `Makefile` | Automates compilation and execution of the supplied test suite. |
| `test_myalloc.c` | Supplied comprehensive test suite. |
| `test_task3.c` | Tests allocation and reuse functionality. |
| `test_task4.c` | Tests block splitting. |
| `test_task5.c` | Tests freeing and coalescing. |
| `test_calloc.c` | Tests zero-initialized allocation. |
| `test_realloc.c` | Tests memory resizing. |
| `test_policy.c` | Tests the best-fit allocation policy. |
| `test_task7.c` | Tests heap statistics, heap dumping and heap consistency. |
| `bench.c` | Benchmarks the allocation policies against standard C allocation functions. |
| `REPORT.md` | Contains the benchmark methodology, results and analysis. |

## 4. Implementation Progress

The following table summarizes the implementation of the eight assignment tasks.

| Task | Description | Status |
|---|---|---|
| 1 | Block header structure, alignment and helper functions | Completed |
| 2 | Heap growth using `sbrk()` | Completed |
| 3 | First-fit memory allocation | Completed |
| 4 | Splitting free memory blocks | Completed |
| 5 | Memory deallocation and coalescing | Completed |
| 6 | `my_calloc()` and `my_realloc()` | Completed |
| 7 | Best-fit policy, heap statistics, heap dump and consistency checking | Completed |
| 8 | Benchmarking and performance report | Completed |

## 5. Design and Implementation Explanation

Memory blocks are stored in a linked list structure. Each block has a block header, which stores the size of the payload, a pointer to the next block, whether the block is free, and a magic number to allow detection of valid blocks.

Block alignment

All blocks are aligned to 16 byte boundaries. This means that any payload pointers returned by this allocator will be aligned. The block header also has its size rounded up to a multiple of 16, to ensure payload pointers are correctly aligned.

Allocation

Given a size from my_malloc(), the size is rounded up. Depending on the allocation policy, either best or first fit, my_malloc() finds an appropriate block in the free list. If there isn’t a suitable block, more memory is requested from the OS using sbrk().

If large free blocks are found, they will be split if the leftover space would allow storage of a new header, and at least 16 bytes of payload. If memory is freed (using my_free()), adjacent free blocks will be merged (coalesced) to reduce external fragmentation.

Functions

my_calloc() ensures that an overflow has not occurred due to the multiplication, then initialises the memory to 0. my_realloc() allows resizing of blocks, reducing, expanding to fit into adjacent free blocks and moving data if required.

Debugging

Various debugging functions have been implemented to allow for analysis of the heap. This includes returning statistics about the heap, information about blocks, and ensuring that the heap is consistent. This allows observation of how the allocator behaves, and detection of issues in the structure of the heap.
## 6. Final Test Results

The supplied test suite was executed using:

```bash
make clean
make
```

### Final output

```text
T1 basic allocation
  PASS  returns non-NULL
  PASS  16-byte aligned
  PASS  heap consistent
  PASS  malloc(0) is NULL
  PASS  free(NULL) is safe
T2 blocks do not overlap
  PASS  data intact in all blocks
  PASS  heap consistent after frees
T3 reuse and splitting
  PASS  freed space is reused (no growth)
  PASS  heap did not grow
  PASS  block was split (remainder is free)
T4 coalescing
  PASS  all memory is free
  PASS  adjacent free blocks merged into one
  PASS  merged block satisfies a 550-byte request
T5 calloc and realloc
  PASS  calloc zeroes memory
  PASS  calloc overflow returns NULL
  PASS  realloc preserves contents
  PASS  shrinking keeps prefix
  PASS  realloc(NULL, n) acts like malloc
  PASS  realloc(p, 0) frees and returns NULL
  PASS  heap consistent
T6 heap growth via sbrk
  PASS  large request succeeds
  PASS  heap grew
  PASS  heap consistent
T7 best-fit vs first-fit
  PASS  first-fit takes the first hole
  PASS  best-fit takes the tightest hole
T8 randomized stress test
  PASS  no data corruption during 20000 operations
  PASS  everything freed
  PASS  fully coalesced (few free blocks remain)
  PASS  heap consistent

Result: 29 passed, 0 failed
```

**Overall result: 29 tests passed and 0 failed.**

## 7. Benchmark Results

The benchmark performed 50,000 randomized allocation and deallocation operations using allocation sizes between 1 and 4,096 bytes and a fixed random seed of 42.

### First-fit and Best-fit comparison

| Measurement | First-fit | Best-fit |
|---|---:|---:|
| Execution time | 0.130155 s | 0.204763 s |
| Heap bytes | 1,376,928 | 1,376,928 |
| Used bytes | 1,060,016 | 1,060,912 |
| Free bytes | 294,032 | 294,832 |
| Free blocks | 223 | 170 |
| Largest free block | 149,536 | 134,816 |
| External fragmentation | 49.14% | 54.27% |
| Heap consistency | PASS | PASS |

### Comparison with standard C allocation

| Allocator | Execution time |
|---|---:|
| First-fit | 0.130155 seconds |
| Best-fit | 0.204763 seconds |
| Standard C `malloc()`/`free()` | 0.005789 seconds |

The results indicate that first-fit was faster than best-fit for the tested workload. Best-fit produced fewer free blocks but had higher external fragmentation. The standard C allocator achieved the shortest execution time.

For the full benchmark discussion and methodology, refer to `REPORT.md`.

## 8. Answers to Assignment Questions

### Question 1: Why should the header size be a multiple of 16 as well as the payload?

The header size should be a multiple of 16 to ensure that the payload begins at a 16-byte aligned address. Even if the beginning of a block is aligned, an incorrectly sized header could cause the payload to become misaligned. Rounding the header size up to a multiple of 16 preserves alignment for every allocated payload.

### Question 2: Why can calling `printf()` inside `my_malloc()` cause recursion?

The standard `printf()` function may internally request memory from the system allocator. If this happens while `my_malloc()` is executing, it could trigger another allocation request, potentially causing recursive calls or unexpected allocator behaviour. This can lead to infinite recursion or corruption. Using `write()` for output avoids relying on formatted standard I/O during allocator operations.

### Question 3: What additional field could be used to find the previous block in O(1) time, and what is the technique called?

A `prev` pointer can be added to each block header to point to the previous block. This creates a doubly linked list, allowing direct access to the previous block without traversing the entire list. This improves backward traversal and can make finding the previous list node an O(1) operation.

### Question 4: What is the difference between internal and external fragmentation?

Internal fragmentation occurs when an allocated block contains unused space because the allocator rounds requested sizes up for alignment or allocation requirements.

External fragmentation occurs when free memory is divided into multiple separated blocks. Although the total free memory may be sufficient for a request, no individual free block may be large enough to satisfy it.

### Question 5: Why is memory obtained through `sbrk()` not automatically returned to the operating system, and how could it be returned?

Memory obtained through `sbrk()` changes the program's heap boundary. Freeing an individual block does not automatically reduce that boundary because other allocated blocks may exist beyond it. Memory could potentially be returned if the free block is located at the end of the heap. The allocator could shrink the program break using a negative `sbrk()` increment, provided the memory is no longer in use and the allocator controls the affected heap region.

### Question 6: Why can two consecutive blocks in the linked list not be physically adjacent, and what does coalescing do?

Two consecutive blocks in the linked list may not be physically adjacent because the allocator can obtain separate memory regions through multiple `sbrk()` calls, and alignment padding may exist between them. Therefore, linked-list order alone does not guarantee physical adjacency.

Coalescing checks whether neighbouring free blocks are physically adjacent in memory. If they are, their payload spaces and intervening header are combined into one larger free block. This helps reduce external fragmentation.

## 9. Limitations

The implementation has the following limitations:

- It uses `sbrk()`, which is deprecated and is not portable across all operating systems.
- The allocator is not designed to be thread-safe.
- It uses a linked-list search, which can become slower as the number of blocks increases.
- Memory is not generally returned to the operating system when individual blocks are freed.
- The benchmark measures execution time but does not collect equivalent internal heap statistics from the standard C allocator.
- It implements basic allocation policies rather than advanced strategies used by production memory allocators.

## 10. AI Assistance and Source Declaration

AI assistance: ChatGPT was used as a learning and support tool for understanding memory allocation concepts, troubleshooting compilation errors and reviewing implementation logic.
## 11. Conclusion

The project successfully implemented a custom memory allocator in C with support for allocation, deallocation, block splitting, coalescing, resizing, heap growth and two placement policies.

All 29 supplied tests passed, including the randomized stress test. The benchmark also demonstrated measurable differences between first-fit and best-fit allocation strategies and the standard C allocator.

The project provided practical experience in memory management, pointer arithmetic, linked lists, system calls, memory alignment and performance evaluation.
