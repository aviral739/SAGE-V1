# SAGE
## Smart Adaptive Generic Search Engine
### Hardware-Aware Parallel Search Framework for Massive Unsorted Datasets

## Project Overview

SAGE v1 is a C++20 hardware-aware parallel search framework designed for massive unsorted in-memory integer datasets. It does not claim to beat linear search theoretically, but improves real execution time using parallel search, early stopping, and safe worker allocation.

## Why SAGE?

The problem SAGE addresses:
- Sorted data can use binary search with O(log n) complexity
- Unsorted data usually requires linear search with O(n) complexity  
- For massive datasets, linear search becomes slow in practice
- Blindly using all CPU cores can overload the system
- SAGE v1 tries to improve practical search time while keeping resource usage controlled

## SAGE v1 Features

- **Synthetic massive dataset generation**
- **Target placement control**: MIDDLE, BEGINNING, ABSENT
- **Manual linear search baseline**
- **std::find baseline**
- **Parallel chunk-based search**
- **Early termination using atomic flag**
- **Logical CPU core detection**
- **Safe worker recommendation**
- **Benchmark timing using std::chrono**
- **Correctness validation**

## Current Scope of v1

- Only standard C++20
- Only `std::vector<std::int64_t>`
- Only in-memory search
- No SIMD, mmap, GPU, Bloom filters, CSV, dashboard, indexing, or database support yet

## Architecture

```
User Query / Target Key
        ↓
Synthetic Dataset Generator
        ↓
Hardware Detector
        ↓
Safe Worker Allocator
        ↓
Search Methods
   ├── Manual Linear Search
   ├── std::find Baseline
   └── Parallel Early-Stopping Search
        ↓
Benchmark + Correctness Report
```

## Benchmark Results

Test Setup:
- Dataset size: 50,000,000 int64_t elements
- Target value: 9999999937
- Logical cores detected: 12
- Recommended workers: 8

| Target Placement | Linear Search | std::find | Parallel Search | Speedup vs Linear | Correct |
|---|---:|---:|---:|---:|---|
| MIDDLE | 158.608 ms | 127.713 ms | 2.200 ms | 72.09x | YES |
| BEGINNING | 0.001 ms | 0.000 ms | 1.321 ms | 0.00x | YES |
| ABSENT | 313.624 ms | 252.442 ms | 104.087 ms | 3.01x | YES |

## Benchmark Observations

- **MIDDLE case** shows strong speedup because the dataset is split across workers and early stopping reduces unnecessary work.
- **ABSENT case** is important because all methods must scan the full dataset; SAGE still achieved 3.01x speedup.
- **BEGINNING case** is faster with linear search because the target is found immediately at index 0, while parallel search has thread creation overhead.
- This shows SAGE v1 is not meant to replace linear search in every case. It is useful for massive datasets where the search is not trivially resolved immediately.

## Build Instructions

```powershell
$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
cd D:\SAGE
cmake -S . -B build -G "Ninja"
cmake --build build
.\build\sage.exe
```

Normal cross-platform CMake commands:
```bash
cmake -S . -B build
cmake --build build
./build/sage
```

## Project Structure

```
SAGE/
├── CMakeLists.txt
├── README.md
├── main.cpp
├── engine/
│ ├── include/
│ │ ├── dataset_generator.hpp
│ │ ├── search.hpp
│ │ ├── hardware.hpp
│ │ └── benchmark.hpp
│ └── src/
│ ├── dataset_generator.cpp
│ ├── search.cpp
│ ├── hardware.cpp
│ └── benchmark.cpp
└── benchmarks/
└── results/
└── benchmark_50M_v1.txt
```

## Roadmap

### SAGE v1
- Hardware-aware parallel search core
- Linear/std::find/parallel benchmarks
- Early stopping
- Synthetic dataset generation

### SAGE v2
- Memory-safe chunking
- CSV / binary file support
- Block metadata
- Min-max pruning
- Benchmark dashboard

### SAGE v3
- SIMD acceleration
- Memory-mapped search
- Work-stealing thread pool
- Bloom filters
- Profiling and flamegraphs

### SAGE v4
- Optional GPU offloading
- Approximate search
- Online strategy tuning
- Distributed search simulation

## Limitations

- v1 works only with in-memory integer vectors
- Parallel search has overhead and may be slower when the target is at the beginning
- Results depend on hardware, CPU load, compiler, and dataset distribution
- v1 does not replace indexing or hashing for repeated exact lookups

## Final Note

SAGE v1 is the foundation. The goal is to evolve it version by version into a high-performance adaptive search framework for massive unsorted datasets.
