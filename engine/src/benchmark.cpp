#include "benchmark.hpp"
#include <chrono>

namespace sage {

BenchmarkResult benchmark_search(
    const std::string& method_name,
    const std::vector<std::int64_t>& data,
    std::int64_t target,
    std::optional<std::size_t> expected_index,
    const std::function<std::optional<std::size_t>(const std::vector<std::int64_t>&, std::int64_t)>& search_function
) {
    BenchmarkResult result;
    result.method_name = method_name;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    result.result_index = search_function(data, target);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    result.elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.correct = (result.result_index == expected_index);
    
    return result;
}

} // namespace sage
