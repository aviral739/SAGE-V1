#include "hardware.hpp"
#include <thread>

namespace sage {

std::size_t logical_core_count() {
    std::size_t cores = std::thread::hardware_concurrency();
    return (cores == 0) ? 1 : cores;
}

std::size_t recommended_worker_count(std::size_t dataset_size) {
    if (dataset_size == 0) {
        return 1;
    }
    
    const std::size_t cores = logical_core_count();
    const std::size_t target_workers = static_cast<std::size_t>(cores * 0.75);
    
    // Apply constraints: minimum 1, maximum 8 for v1, not more than dataset_size
    std::size_t result = std::max(std::size_t{1}, target_workers);
    result = std::min(result, std::size_t{8}); // v1 limit
    result = std::min(result, dataset_size);
    
    return result;
}

} // namespace sage
