#pragma once

#include <vector>
#include <cstdint>
#include <optional>

namespace sage {

std::optional<std::size_t> linear_search(
    const std::vector<std::int64_t>& data,
    std::int64_t target
);

std::optional<std::size_t> std_find_search(
    const std::vector<std::int64_t>& data,
    std::int64_t target
);

std::optional<std::size_t> parallel_search(
    const std::vector<std::int64_t>& data,
    std::int64_t target,
    std::size_t worker_count
);

} // namespace sage
