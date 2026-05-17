# SAGE — Smart Adaptive Generic Search Engine
## A C++20 resource-aware adaptive search engine for massive unsorted datasets.

## Short Overview

SAGE is a C++20 performance-engineering project focused on practical search over massive unsorted datasets. It starts from simple baseline search, then evolves into hardware-aware parallel search, adaptive metadata-assisted pruning, and SIMD-accelerated scanning.

- **v1** implemented hardware-aware parallel search
- **v2** added metadata-assisted adaptive search
- **v3** adds dynamic work queues, AVX2 SIMD validation, and metadata-pruned SIMD search

## Why SAGE?

Searching massive unsorted datasets is expensive when:
- data is not sorted
- no index exists  
- building an index is not worth the cost
- the dataset is temporary
- memory overhead matters
- occasional searches are required

SAGE improves practical search performance using:
- CPU-aware worker selection
- chunk-based parallel search
- early termination
- block metadata
- metadata-pruned search
- reusable thread pool
- dynamic work queue scheduling
- SIMD-accelerated scanning (AVX2 when available)
- benchmark-based validation

## Core Message

**The goal is not to beat every search algorithm. The goal is to avoid wasting computation.**

SAGE adapts the search strategy to system resources, available logical cores, dataset size, block metadata, target behavior, pruning opportunities, and SIMD availability. It does not replace linear search, binary search, hashing, indexing, or databases.

## Version Status

### SAGE v1.0.0 — Completed
Features:
- C++20 implementation
- Synthetic massive dataset generation
- Target placement control
- Manual linear search baseline
- std::find baseline
- Chunk-based parallel search
- Hardware-aware logical core detection
- Worker recommendation
- Atomic early termination
- Correctness validation
- Benchmarking on up to 50 million int64_t elements

### SAGE v2.0.0 — Completed
Features:
- Resource-aware worker policy
- Chunk manager
- Block-level metadata generation
- Metadata-pruned parallel search
- Simple strategy selector
- Realistic absent-case validation
- CSV benchmark export
- Final raw benchmark output

### SAGE v3.0.0 — Final Release
Features:
- Reusable ThreadPool foundation
- Dynamic Work Queue Search
- Clean v3 benchmark output
- Release-mode benchmarking (-O3)
- AVX2 SIMD Search with scalar fallback
- SIMD mode detection at runtime
- Metadata-Pruned SIMD Search (flagship v3 method)
- Benchmark CSV export updated to v3

## How v3 Works

```
Dataset → Chunks → Block Metadata → Prune Impossible Blocks → SIMD Scan Eligible Blocks → Result + Metrics
```

**Metadata Structure:**
Each block stores:
- block id
- start index
- end index
- min value
- max value
- count

**Pruning Logic:**
If target is outside a block's min/max range, that block can be skipped safely.

**SIMD Scanning:**
Eligible blocks are scanned using AVX2 intrinsics (16 int64_t per unrolled iteration) when available, with automatic scalar fallback. The thread pool processes eligible blocks in parallel, each using SIMD within its assigned range.

**Limitation:**
If the target is within the min/max range of every block, metadata cannot prune and performance depends entirely on the speed of the underlying scan.

## Benchmark Results (v3.0.0 Release Build)

| Case | Linear Search | Fixed Chunk Parallel | Metadata-Pruned Parallel | **Metadata-Pruned SIMD** | Blocks Searched | Blocks Skipped | Prune % |
|------|--------------:|--------------------:|-------------------------:|-------------------------:|----------------:|---------------:|--------:|
| MIDDLE | 9.891 ms | 2.138 ms | 1.400 ms | **2.1 ms** | 1 | 6 | 85.7% |
| ABSENT | 20.008 ms | — | 0.001 ms | **0.0 ms** | 0 | 7 | 100.0% |
| ABSENT_IN_RANGE | 19.515 ms | 14.334 ms | 14.698 ms | **13.1 ms** | 7 | 0 | 0.0% |

| Case | Metadata-Pruned Speedup | **Metadata-Pruned SIMD Speedup** |
|------|------------------------:|----------------------------------:|
| MIDDLE | 7.07x | **4.80x** |
| ABSENT | 20008.00x | **40016.40x** |
| ABSENT_IN_RANGE | 1.33x | **1.49x** |

Standalone SIMD Search is included and validated for correctness. In Release mode, compiler-optimized scalar search is already highly competitive, so standalone SIMD does not always show dramatic speedup. The strongest v3 SIMD result comes from Metadata-Pruned SIMD Search, where SIMD is applied only after metadata pruning selects eligible blocks.

**Dataset:** 50,000,000 int64_t elements  
**Build:** Release (-O3), AVX2 enabled  
**Hardware:** 12 logical cores detected  
**Resource mode:** BALANCED  
**Adaptive workers:** 7

## Benchmark Interpretation

- **MIDDLE case:** Metadata pruning eliminates 6 of 7 blocks, leaving only 1 block (~7M elements) to search. The metadata-pruned parallel search is fastest here because the single eligible block is split across threads. Metadata-Pruned SIMD still achieves 4.80x speedup over linear.

- **ABSENT case:** Out-of-range absent target — metadata skips all 7 blocks, both metadata-pruned methods complete in sub-millisecond time. The huge speedup numbers reflect near-instant completion and should not be treated as universal.

- **ABSENT_IN_RANGE:** The realistic hard absent case. The target value (500,000) is within the range of every block, so metadata cannot skip any block. All 50M elements must be scanned. This is where **Metadata-Pruned SIMD Search** proves its value — SIMD accelerates the scan within each of the 7 parallel blocks, achieving the best absolute time (13.1 ms) among all methods.

- **Standalone SIMD Search** is included and validated but compiler-optimized scalar search at -O3 is already competitive. The strongest v3 method is **Metadata-Pruned SIMD Search** because SIMD is applied only after pruning, combining the best of both techniques.

**Important:** The enormous ABSENT metadata speedup applies only to out-of-range pruning cases. Real workloads typically resemble ABSENT_IN_RANGE, where honest trade-offs apply.

## Build and Run

### PowerShell (MSYS2 UCRT64)
```powershell
$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
cd D:\SAGE
cmake -S . -B build
cmake --build build
.\build\sage.exe
```

### Cross-platform CMake
```bash
cmake -S . -B build
cmake --build build
./build/sage
```

AVX2 is enabled by default. To disable (e.g., for non-AVX2 CPUs):
```bash
cmake -S . -B build -DSAGE_ENABLE_AVX2=OFF
```

## Project Structure

```
SAGE/
├── CMakeLists.txt
├── README.md
├── main.cpp
├── engine/
│   ├── include/
│   │   ├── dataset_generator.hpp
│   │   ├── search.hpp
│   │   ├── hardware.hpp
│   │   ├── benchmark.hpp
│   │   ├── resource_policy.hpp
│   │   ├── chunk_manager.hpp
│   │   ├── metadata.hpp
│   │   ├── strategy.hpp
│   │   ├── csv_export.hpp
│   │   └── thread_pool.hpp
│   └── src/
│       ├── dataset_generator.cpp
│       ├── search.cpp
│       ├── hardware.cpp
│       ├── benchmark.cpp
│       ├── resource_policy.cpp
│       ├── chunk_manager.cpp
│       ├── metadata.cpp
│       ├── strategy.cpp
│       ├── csv_export.cpp
│       └── thread_pool.cpp
├── benchmarks/
│   └── results/
│       ├── benchmark_50M_v1.txt
│       ├── benchmark_50M_v2.txt
│       ├── benchmark_50M_v3.txt
│       ├── v2_results.csv
│       └── v3_results.csv
└── build/
    ├── sage.exe
    └── ... (build artifacts)
```

## Limitations

- Works only with in-memory integer vectors
- Parallel search has overhead and may be slower when the target is at the beginning
- Results depend on hardware, CPU load, compiler, and dataset distribution
- Does not replace indexing or hashing for repeated exact lookups
- Metadata pruning effectiveness depends on data distribution
- SIMD acceleration requires an AVX2-capable x86 CPU; falls back to scalar on other architectures
- Release-mode compiler optimizations (-O3) narrow the gap between hand-tuned SIMD and auto-vectorized scalar code

## Project Status

**SAGE v3.0.0 is the final release phase of this project.** There will be no v4. The project demonstrates the evolution of search strategy from simple linear scan to hardware-aware parallel search, metadata-assisted pruning, and SIMD-accelerated adaptive search. It is useful when sorting or building a full index may be expensive, unavailable, memory-heavy, temporary, or unnecessary for one-time or occasional searches. It provides a practical performance improvement for massive unsorted dataset search without claiming to replace all search algorithms or indexing systems.
