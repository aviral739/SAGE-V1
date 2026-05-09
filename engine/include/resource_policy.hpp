#pragma once

#include <cstddef>

namespace sage {

enum class ResourceMode {
    ECO,
    BALANCED,
    PERFORMANCE,
    MAX
};

struct ResourcePolicy {
    ResourceMode mode;
    std::size_t logical_cores;
    std::size_t recommended_workers;
    std::size_t max_workers;
};

std::size_t recommended_worker_count(std::size_t dataset_size, ResourceMode mode);

const char* to_string(ResourceMode mode);

} // namespace sage
