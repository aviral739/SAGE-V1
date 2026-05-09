#include "dataset_generator.hpp"
#include "search.hpp"
#include "hardware.hpp"
#include "benchmark.hpp"
#include <iostream>
#include <iomanip>

void run_benchmark_case(const std::string& case_name, sage::TargetPlacement placement) {
    std::cout << "===================================================\n";
    std::cout << "SAGE v1 Benchmark Case: " << case_name << "\n";
    std::cout << "===================================================\n";
    
    // Configuration
    constexpr std::size_t dataset_size = 50'000'000;
    
    // Generate dataset
    std::cout << "Generating dataset...\n";
    auto dataset = sage::generate_dataset(dataset_size, placement);
    
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
    
    // Speedup calculation
    double speedup = linear_result.elapsed_ms / parallel_result.elapsed_ms;
    std::cout << "Parallel search speedup: " << std::setprecision(2) << speedup << "x\n\n";
}

int main() {
    std::cout << "SAGE v1 - Hardware-Aware Parallel Search Framework\n";
    std::cout << "===================================================\n\n";
    
    // Run benchmark cases
    run_benchmark_case("MIDDLE", sage::TargetPlacement::MIDDLE);
    run_benchmark_case("BEGINNING", sage::TargetPlacement::BEGINNING);
    run_benchmark_case("ABSENT", sage::TargetPlacement::ABSENT);
    
    return 0;
}
