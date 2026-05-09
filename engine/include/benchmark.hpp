#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <functional>

namespace sage {

struct BenchmarkResult {
    std::string method_name;
    std::optional<std::size_t> result_index;
    double elapsed_ms;
    bool correct;
};

BenchmarkResult benchmark_search(
    const std::string& method_name,
    const std::vector<std::int64_t>& data,
    std::int64_t target,
    std::optional<std::size_t> expected_index,
    const std::function<std::optional<std::size_t>(const std::vector<std::int64_t>&, std::int64_t)>& search_function
);

} // namespace sage
