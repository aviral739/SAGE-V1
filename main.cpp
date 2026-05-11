#include "dataset_generator.hpp"
#include "search.hpp"
#include "hardware.hpp"
#include "benchmark.hpp"
#include "resource_policy.hpp"
#include "chunk_manager.hpp"
#include "metadata.hpp"
#include "strategy.hpp"
#include "csv_export.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <filesystem>

void run_benchmark_case(const std::string& case_name, sage::TargetPlacement placement) {
    std::cout << "===================================================\n";
    std::cout << "SAGE v2 Benchmark Case: " << case_name << "\n";
    std::cout << "===================================================\n";
    
    // Configuration
    constexpr std::size_t dataset_size = 50'000'000;
    
    // Generate dataset
    std::cout << "Generating dataset...\n";
    auto dataset = sage::generate_dataset(dataset_size, placement);
    
    // SAGE v2 Diagnostic
    sage::ResourceMode mode = sage::ResourceMode::BALANCED;
    std::size_t v2_workers = sage::recommended_worker_count(dataset_size, mode);
    auto chunks = sage::create_chunks(dataset_size, v2_workers);
    bool chunks_valid = sage::validate_chunks(chunks, dataset_size);
    auto metadata = sage::generate_metadata(dataset.data, chunks);
    auto metadata_summary = sage::summarize_metadata(metadata);
    auto strategy = sage::choose_strategy(dataset_size, true, mode);
    
    std::cout << "SAGE v2 Diagnostic:\n";
    std::cout << "  Resource mode: " << sage::to_string(mode) << "\n";
    std::cout << "  V2 recommended workers: " << v2_workers << "\n";
    std::cout << "  Chunks created: " << chunks.size() << "\n";
    std::cout << "  Chunks valid: " << (chunks_valid ? "YES" : "NO") << "\n";
    std::cout << "  Metadata blocks: " << metadata_summary.total_blocks << "\n";
    std::cout << "  Metadata total elements: " << metadata_summary.total_elements << "\n";
    std::cout << "  Selected strategy: " << sage::to_string(strategy.strategy) << "\n";
    std::cout << "  Strategy reason: " << strategy.reason << "\n\n";
    
    // Hardware detection
    const std::size_t logical_cores = sage::logical_core_count();
    const std::size_t recommended_workers = sage::recommended_worker_count(dataset_size);
    
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
    std::cout << "\n";
    
    std::cout << "Logical cores: " << logical_cores << "\n";
    std::cout << "Recommended workers: " << recommended_workers << "\n\n";
    
    // Benchmark results
    std::cout << "Benchmark Results:\n";
    std::cout << "-----------------\n";
    
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
        "Parallel Search",
        dataset.data,
        dataset.target,
        dataset.expected_index,
        [recommended_workers](const std::vector<std::int64_t>& data, std::int64_t target) {
            return sage::parallel_search(data, target, recommended_workers);
        }
    );
    
    // Metadata-pruned parallel search
    auto start_time = std::chrono::high_resolution_clock::now();
    auto metadata_result = sage::metadata_pruned_parallel_search(
        dataset.data,
        dataset.target,
        metadata,
        v2_workers
    );
    auto end_time = std::chrono::high_resolution_clock::now();
    double metadata_elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    bool metadata_correct = (metadata_result.result_index == dataset.expected_index);
    double metadata_skip_percentage = (metadata_result.total_blocks > 0) ? 
        (static_cast<double>(metadata_result.blocks_skipped) / metadata_result.total_blocks * 100.0) : 0.0;
    
    // Display results
    std::cout << std::fixed << std::setprecision(3);
    
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
    
    // Speedup calculations
    std::cout << std::setprecision(2);
    double parallel_speedup = linear_result.elapsed_ms / parallel_result.elapsed_ms;
    double metadata_speedup = linear_result.elapsed_ms / metadata_elapsed_ms;
    std::cout << "Parallel search speedup: " << parallel_speedup << "x\n";
    std::cout << "Metadata-pruned search speedup: " << metadata_speedup << "x\n\n";
    
    // Export to CSV
    sage::BenchmarkCsvRow csv_row;
    csv_row.version = "v2";
    csv_row.placement = case_name;
    csv_row.dataset_size = dataset_size;
    csv_row.target_value = dataset.target;
    csv_row.resource_mode = sage::to_string(mode);
    csv_row.selected_strategy = sage::to_string(strategy.strategy);
    csv_row.logical_cores = logical_cores;
    csv_row.workers = v2_workers;
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
    
    const std::string csv_path = "benchmarks/results/v2_results.csv";
    sage::write_csv_header_if_needed(csv_path);
    sage::append_benchmark_row(csv_path, csv_row);
    
    std::cout << "CSV row exported: " << csv_path << "\n\n";
}

int main() {
    std::cout << "SAGE v2 - Adaptive Metadata-Assisted Search Framework\n";
    std::cout << "===================================================\n\n";
    
    // Delete old CSV file to ensure fresh start
    const std::string csv_path = "benchmarks/results/v2_results.csv";
    std::filesystem::remove(csv_path);
    
    // Run benchmark cases
    run_benchmark_case("MIDDLE", sage::TargetPlacement::MIDDLE);
    run_benchmark_case("BEGINNING", sage::TargetPlacement::BEGINNING);
    run_benchmark_case("ABSENT", sage::TargetPlacement::ABSENT);
    run_benchmark_case("ABSENT_IN_RANGE", sage::TargetPlacement::ABSENT_IN_RANGE);
    
    return 0;
}
