# SAGE — Smart Adaptive Generic Search Engine
## Hardware-Aware Adaptive Search Framework for Massive Unsorted Datasets

## Short Overview

SAGE is a C++20 performance-engineering project focused on practical search over massive unsorted datasets. It starts from simple baseline search, then evolves into hardware-aware parallel search and adaptive metadata-assisted pruning.

- **v1** implemented hardware-aware parallel search
- **v2** adds metadata-assisted adaptive search

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
- benchmark-based validation

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

### SAGE v2.0.0 — Current
Features:
- Resource-aware worker policy
- Chunk manager
- Block-level metadata generation
- Metadata-pruned parallel search
- Simple strategy selector
- Realistic absent-case validation
- CSV benchmark export
- Final raw benchmark output

## How v2 Works

```
Dataset -> Chunks -> Block Metadata -> Strategy Selector -> Metadata-Pruned Parallel Search -> Result + Metrics
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

**Limitation:**
If the target is within the min/max range of every block, metadata cannot prune and overhead may make metadata-pruned search slightly slower than normal parallel search.

## Benchmark Results

| Case | Linear Search | std::find | Parallel Search | Metadata-Pruned Search | Blocks Searched | Blocks Skipped | Parallel Speedup | Metadata Speedup |
|------|--------------:|----------:|----------------:|-----------------------:|----------------:|---------------:|-----------------:|-----------------:|
| MIDDLE | 144.125 ms | 118.288 ms | 2.508 ms | 6.786 ms | 1 | 6 | 57.48x | 21.24x |
| BEGINNING | 0.001 ms | 0.000 ms | 1.888 ms | 1.732 ms | 1 | 6 | 0.00x | 0.00x |
| ABSENT | 296.608 ms | 239.979 ms | 94.861 ms | 0.003 ms | 0 | 7 | 3.13x | 102278.79x |
| ABSENT_IN_RANGE | 300.052 ms | 241.534 ms | 105.887 ms | 105.713 ms | 7 | 0 | 2.83x | 2.84x |

**Dataset:** 50,000,000 int64_t elements  
**Hardware:** 12 logical cores detected  
**Resource mode:** BALANCED  
**Recommended v2 workers:** 7

## Benchmark Interpretation

- **MIDDLE case** shows strong benefit from parallel search and metadata pruning because only 1 out of 7 blocks needed to be searched.
- **BEGINNING case** shows that linear search can still win when the target is at the first index because parallelism has overhead.
- **ABSENT case** is an out-of-range absent target, where metadata skips all blocks and search completes almost instantly.
- **ABSENT_IN_RANGE** is the realistic hard absent case. Since the target is within the possible value range, metadata cannot skip any block. This proves the system is honest and validates the trade-off.

**Important:** The huge ABSENT metadata speedup should not be treated as universal. It applies to out-of-range pruning cases.

## Build and Run

```powershell
$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path
cd D:\SAGE
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
│ │ ├── benchmark.hpp
│ │ ├── resource_policy.hpp
│ │ ├── chunk_manager.hpp
│ │ ├── metadata.hpp
│ │ ├── strategy.hpp
│ │ └── csv_export.hpp
│ └── src/
│ ├── dataset_generator.cpp
│ ├── search.cpp
│ ├── hardware.cpp
│ ├── benchmark.cpp
│ ├── resource_policy.cpp
│ ├── chunk_manager.cpp
│ ├── metadata.cpp
│ ├── strategy.cpp
│ └── csv_export.cpp
└── benchmarks/
└── results/
├── v2_results.csv
└── benchmark_50M_v2.txt
```

## Limitations

- Works only with in-memory integer vectors
- Parallel search has overhead and may be slower when the target is at the beginning
- Results depend on hardware, CPU load, compiler, and dataset distribution
- Does not replace indexing or hashing for repeated exact lookups
- Metadata pruning effectiveness depends on data distribution

## Final Note

SAGE is useful when sorting or building a full index may be expensive, unavailable, memory-heavy, temporary, or unnecessary for one-time or occasional searches. It provides a practical performance improvement for massive unsorted dataset search without claiming to replace all search algorithms or indexing systems.
