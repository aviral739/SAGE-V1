#include "dataset_generator.hpp"
#include "search.hpp"
#include "hardware.hpp"
#include "benchmark.hpp"
#include "resource_policy.hpp"
#include "chunk_manager.hpp"
#include "metadata.hpp"
#include "strategy.hpp"
#include "csv_export.hpp"
#include "thread_pool.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <filesystem>

void run_benchmark_case(
    const std::string& case_name,
    sage::TargetPlacement placement,
    std::size_t logical_cores,
    std::size_t recommended_workers,
    std::size_t adaptive_workers,
    sage::ResourceMode mode
) {
    std::cout << "===================================================\n";
    std::cout << "SAGE v3 Benchmark Case: " << case_name << "\n";
    std::cout << "===================================================\n";
    
    // Configuration
    constexpr std::size_t dataset_size = 50'000'000;
    
    // Generate dataset
    std::cout << "Generating dataset...\n";
    auto dataset = sage::generate_dataset(dataset_size, placement);
    
    // Build metadata and strategy
    auto chunks = sage::create_chunks(dataset_size, adaptive_workers);
    auto metadata = sage::generate_metadata(dataset.data, chunks);
    auto metadata_summary = sage::summarize_metadata(metadata);
    auto strategy = sage::choose_strategy(dataset_size, true, mode);
    
    std::cout << "Case Configuration:\n";
    std::cout << "  Workers: " << recommended_workers << "\n";
    std::cout << "  Adaptive workers: " << adaptive_workers << "\n";
    std::cout << "  Blocks: " << metadata_summary.total_blocks << "\n";
    std::cout << "  Strategy: " << sage::to_string(strategy.strategy) << "\n\n";
    
    // Display dataset information
    std::cout << "Dataset size: " << dataset.data.size() << " elements\n";
    std::cout << "Target value: " << dataset.target << "\n";
    std::cout << "Target placement: ";
    switch (dataset.placement) {
        case sage::TargetPlacement::BEGINNING: std::cout << "BEGINNING"; break;
        case sage::TargetPlacement::MIDDLE: std::cout << "MIDDLE"; break;
        case sage::TargetPlacement::END: std::cout << "END"; break;
        case sage::TargetPlacement::RANDOM: std::cout << "RANDOM"; break;
        case sage::TargetPlacement::ABSENT: std::cout << "ABSENT"; break;
        case sage::TargetPlacement::ABSENT_IN_RANGE: std::cout << "ABSENT_IN_RANGE"; break;
    }
    std::cout << "\n";
    
    std::cout << "Expected index: ";
    if (dataset.expected_index) {
        std::cout << *dataset.expected_index;
    } else {
        std::cout << "none (target absent)";
    }
    std::cout << "\n\n";
    
    // Benchmark results
    std::cout << "Benchmark Results:\n";
    std::cout << "-----------------\n\n";
    
    // Linear search
    auto linear_result = sage::benchmark_search(
        "Linear Search",
        dataset.data,
        dataset.target,
        dataset.expected_index,
        [](const std::vector<std::int64_t>& data, std::int64_t target) {
            return sage::linear_search(data, target);
        }
    );
    
    // std::find search
    auto std_find_result = sage::benchmark_search(
        "std::find",
        dataset.data,
        dataset.target,
        dataset.expected_index,
        [](const std::vector<std::int64_t>& data, std::int64_t target) {
            return sage::std_find_search(data, target);
        }
    );
    
    // Parallel search
    auto parallel_result = sage::benchmark_search(
        "Fixed Chunk Parallel Search",
        dataset.data,
        dataset.target,
        dataset.expected_index,
        [recommended_workers](const std::vector<std::int64_t>& data, std::int64_t target) {
            return sage::parallel_search(data, target, recommended_workers);
        }
    );
    
    // Dynamic Parallel Search (v3)
    constexpr std::size_t block_multiplier = 4;
    std::size_t block_count = recommended_workers * block_multiplier;
    auto dynamic_result = sage::benchmark_search(
        "Dynamic Work Queue Search",
        dataset.data,
        dataset.target,
        dataset.expected_index,
        [recommended_workers, block_count](const std::vector<std::int64_t>& data, std::int64_t target) {
            return sage::dynamic_parallel_search(data, target, recommended_workers, block_count);
        }
    );
    
    // Metadata-pruned parallel search
    auto start_time = std::chrono::high_resolution_clock::now();
    auto metadata_result = sage::metadata_pruned_parallel_search(
        dataset.data,
        dataset.target,
        metadata,
        adaptive_workers
    );
    auto end_time = std::chrono::high_resolution_clock::now();
    double metadata_elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    bool metadata_correct = (metadata_result.result_index == dataset.expected_index);
    double metadata_skip_percentage = (metadata_result.total_blocks > 0) ? 
        (static_cast<double>(metadata_result.blocks_skipped) / metadata_result.total_blocks * 100.0) : 0.0;
    
    // Metadata-pruned SIMD search
    auto metadata_simd_start = std::chrono::high_resolution_clock::now();
    auto metadata_simd_result = sage::metadata_pruned_simd_search(
        dataset.data,
        dataset.target,
        metadata,
        adaptive_workers
    );
    auto metadata_simd_end = std::chrono::high_resolution_clock::now();
    double metadata_simd_elapsed_ms = std::chrono::duration<double, std::milli>(metadata_simd_end - metadata_simd_start).count();
    bool metadata_simd_correct = (metadata_simd_result.result_index == dataset.expected_index);
    double metadata_simd_skip_percentage = (metadata_simd_result.total_blocks > 0) ? 
        (static_cast<double>(metadata_simd_result.blocks_skipped) / metadata_simd_result.total_blocks * 100.0) : 0.0;
    
    // Display results
    std::cout << std::fixed << std::setprecision(3);
    
    std::cout << "Baseline Methods:\n";
    std::cout << linear_result.method_name << ":\n";
    std::cout << "  Result index: ";
    if (linear_result.result_index) {
        std::cout << *linear_result.result_index;
    } else {
        std::cout << "not found";
    }
    std::cout << "\n";
    std::cout << "  Time: " << linear_result.elapsed_ms << " ms\n";
    std::cout << "  Correct: " << (linear_result.correct ? "YES" : "NO") << "\n\n";
    
    std::cout << std_find_result.method_name << ":\n";
    std::cout << "  Result index: ";
    if (std_find_result.result_index) {
        std::cout << *std_find_result.result_index;
    } else {
        std::cout << "not found";
    }
    std::cout << "\n";
    std::cout << "  Time: " << std_find_result.elapsed_ms << " ms\n";
    std::cout << "  Correct: " << (std_find_result.correct ? "YES" : "NO") << "\n\n";
    
    std::cout << "Parallel Methods:\n";
    std::cout << parallel_result.method_name << ":\n";
    std::cout << "  Result index: ";
    if (parallel_result.result_index) {
        std::cout << *parallel_result.result_index;
    } else {
        std::cout << "not found";
    }
    std::cout << "\n";
    std::cout << "  Time: " << parallel_result.elapsed_ms << " ms\n";
    std::cout << "  Correct: " << (parallel_result.correct ? "YES" : "NO") << "\n\n";
    
    std::cout << dynamic_result.method_name << ":\n";
    std::cout << "  Result index: ";
    if (dynamic_result.result_index) {
        std::cout << *dynamic_result.result_index;
    } else {
        std::cout << "not found";
    }
    std::cout << "\n";
    std::cout << "  Time: " << dynamic_result.elapsed_ms << " ms\n";
    std::cout << "  Correct: " << (dynamic_result.correct ? "YES" : "NO") << "\n\n";
    
    std::cout << "SIMD Methods:\n";
    // SIMD Search
    auto simd_result = sage::benchmark_search(
        "SIMD Search",
        dataset.data,
        dataset.target,
        dataset.expected_index,
        [](const std::vector<std::int64_t>& data, std::int64_t target) {
            return sage::simd_search(data, target);
        }
    );

    std::cout << simd_result.method_name << ":\n";
    std::cout << "  Result index: ";
    if (simd_result.result_index) {
        std::cout << *simd_result.result_index;
    } else {
        std::cout << "not found";
    }
    std::cout << "\n";
    std::cout << "  Time: " << simd_result.elapsed_ms << " ms\n";
    std::cout << "  Correct: " << (simd_result.correct ? "YES" : "NO") << "\n\n";

    std::cout << "Adaptive Methods:\n";
    // Metadata-Pruned Parallel Search
    std::cout << "Metadata-Pruned Parallel Search:\n";
    std::cout << "  Result index: ";
    if (metadata_result.result_index) {
        std::cout << *metadata_result.result_index;
    } else {
        std::cout << "not found";
    }
    std::cout << "\n";
    std::cout << "  Time: " << metadata_elapsed_ms << " ms\n";
    std::cout << "  Correct: " << (metadata_correct ? "YES" : "NO") << "\n";
    std::cout << "  Total blocks: " << metadata_result.total_blocks << "\n";
    std::cout << "  Blocks searched: " << metadata_result.blocks_searched << "\n";
    std::cout << "  Blocks skipped: " << metadata_result.blocks_skipped << "\n";
    std::cout << "  Skip percentage: " << std::setprecision(1) << metadata_skip_percentage << "%\n\n";
    
    std::cout << "Metadata-Pruned SIMD Search:\n";
    std::cout << "  Result index: ";
    if (metadata_simd_result.result_index) {
        std::cout << *metadata_simd_result.result_index;
    } else {
        std::cout << "not found";
    }
    std::cout << "\n";
    std::cout << "  Time: " << metadata_simd_elapsed_ms << " ms\n";
    std::cout << "  Correct: " << (metadata_simd_correct ? "YES" : "NO") << "\n";
    std::cout << "  Total blocks: " << metadata_simd_result.total_blocks << "\n";
    std::cout << "  Blocks searched: " << metadata_simd_result.blocks_searched << "\n";
    std::cout << "  Blocks skipped: " << metadata_simd_result.blocks_skipped << "\n";
    std::cout << "  Skip percentage: " << std::setprecision(1) << metadata_simd_skip_percentage << "%\n\n";
    
    // Speedup calculations
    std::cout << std::setprecision(2);
    double parallel_speedup = 0.0;
    double dynamic_speedup = 0.0;
    double metadata_speedup = 0.0;
    double simd_speedup = 0.0;
    double metadata_simd_speedup = 0.0;
    if (linear_result.elapsed_ms < 0.01) {
        std::cout << "Fixed chunk parallel speedup: N/A (baseline too small)\n";
        std::cout << "Dynamic work queue speedup: N/A (baseline too small)\n";
        std::cout << "Metadata-pruned speedup: N/A (baseline too small)\n";
        std::cout << "SIMD speedup: N/A (baseline too small)\n";
        std::cout << "Metadata-pruned SIMD speedup: N/A (baseline too small)\n\n";
    } else {
        parallel_speedup = linear_result.elapsed_ms / parallel_result.elapsed_ms;
        dynamic_speedup = linear_result.elapsed_ms / dynamic_result.elapsed_ms;
        metadata_speedup = linear_result.elapsed_ms / metadata_elapsed_ms;
        simd_speedup = linear_result.elapsed_ms / simd_result.elapsed_ms;
        metadata_simd_speedup = linear_result.elapsed_ms / metadata_simd_elapsed_ms;
        std::cout << "Fixed chunk parallel speedup: " << parallel_speedup << "x\n";
        std::cout << "Dynamic work queue speedup: " << dynamic_speedup << "x\n";
        std::cout << "Metadata-pruned speedup: " << metadata_speedup << "x\n";
        std::cout << "SIMD speedup: " << simd_speedup << "x\n";
        std::cout << "Metadata-pruned SIMD speedup: " << metadata_simd_speedup << "x\n\n";
    }
    
    // Export to CSV
    sage::BenchmarkCsvRow csv_row;
    csv_row.version = "v2";
    csv_row.placement = case_name;
    csv_row.dataset_size = dataset_size;
    csv_row.target_value = dataset.target;
    csv_row.resource_mode = sage::to_string(mode);
    csv_row.selected_strategy = sage::to_string(strategy.strategy);
    csv_row.logical_cores = logical_cores;
    csv_row.workers = adaptive_workers;
    csv_row.total_blocks = metadata_result.total_blocks;
    csv_row.blocks_searched = metadata_result.blocks_searched;
    csv_row.blocks_skipped = metadata_result.blocks_skipped;
    csv_row.skip_percentage = metadata_skip_percentage;
    csv_row.linear_ms = linear_result.elapsed_ms;
    csv_row.std_find_ms = std_find_result.elapsed_ms;
    csv_row.parallel_ms = parallel_result.elapsed_ms;
    csv_row.metadata_pruned_ms = metadata_elapsed_ms;
    csv_row.parallel_speedup = parallel_speedup;
    csv_row.metadata_pruned_speedup = metadata_speedup;
    csv_row.linear_correct = linear_result.correct;
    csv_row.std_find_correct = std_find_result.correct;
    csv_row.parallel_correct = parallel_result.correct;
    csv_row.metadata_pruned_correct = metadata_correct;
    
    const std::string csv_path = "benchmarks/results/v3_results.csv";
    sage::write_csv_header_if_needed(csv_path);
    sage::append_benchmark_row(csv_path, csv_row);
    
    std::cout << "CSV row exported: " << csv_path << "\n\n";
}

int main() {
    constexpr std::size_t dataset_size = 50'000'000;
    const std::size_t logical_cores = sage::logical_core_count();
    const std::size_t recommended_workers = sage::recommended_worker_count(dataset_size);
    const sage::ResourceMode mode = sage::ResourceMode::BALANCED;
    const std::size_t adaptive_workers = sage::recommended_worker_count(dataset_size, mode);
    
    std::cout << "SAGE v3 - High-Performance Adaptive Search Framework\n";
    std::cout << "=====================================================\n\n";
    std::cout << "SAGE Runtime Configuration:\n";
    std::cout << "  Version: v3-development\n";
    std::cout << "  Resource mode: " << sage::to_string(mode) << "\n";
    std::cout << "  Logical cores: " << logical_cores << "\n";
    std::cout << "  Recommended workers: " << recommended_workers << "\n";
    std::cout << "  Adaptive workers: " << adaptive_workers << "\n";
    std::cout << "  Thread pool workers: " << adaptive_workers << "\n";
    std::cout << "  SIMD mode: " << sage::simd_mode() << "\n\n";
    
    // Delete old CSV file to ensure fresh start
    const std::string csv_path = "benchmarks/results/v3_results.csv";
    std::filesystem::remove(csv_path);
    
    // Run benchmark cases
    run_benchmark_case("MIDDLE", sage::TargetPlacement::MIDDLE, logical_cores, recommended_workers, adaptive_workers, mode);
    run_benchmark_case("BEGINNING", sage::TargetPlacement::BEGINNING, logical_cores, recommended_workers, adaptive_workers, mode);
    run_benchmark_case("ABSENT", sage::TargetPlacement::ABSENT, logical_cores, recommended_workers, adaptive_workers, mode);
    run_benchmark_case("ABSENT_IN_RANGE", sage::TargetPlacement::ABSENT_IN_RANGE, logical_cores, recommended_workers, adaptive_workers, mode);
    
    return 0;
}
