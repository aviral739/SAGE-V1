#include "resource_policy.hpp"
#include <thread>
#include <algorithm>
#include <stdexcept>

namespace sage {

std::size_t recommended_worker_count(std::size_t dataset_size, ResourceMode mode) {
    if (dataset_size == 0) {
        return 1;
    }
    
    std::size_t logical_cores = std::thread::hardware_concurrency();
    if (logical_cores == 0) {
        logical_cores = 1;
    }
    
    std::size_t target_workers = 0;
    switch (mode) {
        case ResourceMode::ECO:
            target_workers = static_cast<std::size_t>(logical_cores * 0.25);
            break;
        case ResourceMode::BALANCED:
            target_workers = static_cast<std::size_t>(logical_cores * 0.625); // 50% to 75% average
            break;
        case ResourceMode::PERFORMANCE:
            target_workers = static_cast<std::size_t>(logical_cores * 0.825); // 75% to 90% average
            break;
        case ResourceMode::MAX:
            target_workers = logical_cores;
            break;
    }
    
    // Apply constraints
    std::size_t result = std::max(std::size_t{1}, target_workers);
    result = std::min(result, std::size_t{12}); // v2 max worker cap
    result = std::min(result, dataset_size);
    
    return result;
}

const char* to_string(ResourceMode mode) {
    switch (mode) {
        case ResourceMode::ECO: return "ECO";
        case ResourceMode::BALANCED: return "BALANCED";
        case ResourceMode::PERFORMANCE: return "PERFORMANCE";
        case ResourceMode::MAX: return "MAX";
        default: return "UNKNOWN";
    }
}

} // namespace sage
