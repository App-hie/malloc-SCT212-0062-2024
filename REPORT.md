# MEMORY ALLOCATOR BENCHMARK REPORT

## 1. Overview

A memory allocator has been built with support for best-fit and first-fit strategies. Memory is allocated from the OS using the sbrk() function.

The aim of this benchmark is to compare the performance of best-fit and first-fit strategies based on run-time, memory usage and external fragmentation. Additionally, the performance of the custom memory allocator is compared to the standard malloc/free functions provided by the C standard library.
## 2. Benchmark Methodology

The benchmark performed 50,000 randomized allocation and deallocation operations using a fixed random seed of 42 to ensure reproducibility.

The following conditions were used:

- Number of operations: 50,000.
- Allocation sizes: 1–4,096 bytes.
- Number of allocation slots: 1,000.
- Random seed: 42.
- Timing method: `clock_gettime()` with `CLOCK_MONOTONIC`.

Allocation approaches:

1. First-fit custom allocator.
2. Best-fit custom allocator.
3. Standard C library `malloc()` and `free()`.

The custom allocator's heap statistics were collected using `heap_stats()`. External fragmentation was calculated using:

F = 1 - [\frac{\text{Largest Free Block}}{\text{Total Free Memory}}]

## 3. Benchmark Results

### 3.1 First-fit and Best-fit Comparison

| Measurement            |  First-fit |   Best-fit |
| ---------------------- | ---------: | ---------: |
| Execution time         | 0.130155 s | 0.204763 s |
| Heap bytes             |  1,376,928 |  1,376,928 |
| Used bytes             |  1,060,016 |  1,060,912 |
| Free bytes             |    294,032 |    294,832 |
| Free blocks            |        223 |        170 |
| Largest free block     |    149,536 |    134,816 |
| External fragmentation |     49.14% |     54.27% |
| Heap consistency       |       PASS |       PASS |

### 3.2 Comparison with Standard C Allocator

| Allocator              |   Execution time |
| ---------------------- | ---------------: |
| First-fit              | 0.130155 seconds |
| Best-fit               | 0.204763 seconds |
| Standard C malloc/free | 0.005789 seconds |

Heap statistics were not taken for the standard C Allocator since the standard allocation methods of the system were us>

## 4. Discussion and Analysis
The benchmark shows that first-fit performed better than best-fit in execution time, taking approximately 0.13 seconds compared to 0.20 seconds. This is because first-fit stops searching once it finds a suitable free block, while best-fit continues searching for the smallest suitable block.

Best-fit produced fewer free blocks (170) than first-fit (223), but its external fragmentation was higher at 54.27%, compared to 49.14% for first-fit. This demonstrates that fewer free blocks do not necessarily mean less external fragmentation.

The standard C allocator was considerably faster, completing the benchmark in approximately 0.006 seconds. This is expected because standard memory allocators use highly optimized allocation strategies.

Both custom policies passed the heap consistency check, demonstrating that the allocator maintained a valid block structure throughout the benchmark.



## 5. Conclusion

The benchmark successfully demonstrated the operation of both first-fit and best-fit allocation policies. First-fit performed better in execution time and external fragmentation for the tested workload, while best-fit resulted in fewer free blocks. The standard C allocator achieved the best execution time.

 Based on these results, it can be seen that performance varies between allocators depending on workload. It cannot be said for certain that one policy is better than another. Different results may be seen depending on what workload is being performed.
