#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include "metadata.hpp"

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

struct MetadataSearchResult {
    std::optional<std::size_t> result_index;
    std::size_t total_blocks;
    std::size_t blocks_searched;
    std::size_t blocks_skipped;
};

MetadataSearchResult metadata_pruned_parallel_search(
    const std::vector<std::int64_t>& data,
    std::int64_t target,
    const std::vector<BlockMetadata>& metadata,
    std::size_t worker_count
);

} // namespace sage
