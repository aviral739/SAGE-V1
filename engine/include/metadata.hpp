#pragma once

#include "chunk_manager.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>

namespace sage {

struct BlockMetadata {
    std::size_t block_id;
    std::size_t start;
    std::size_t end;
    std::int64_t min_value;
    std::int64_t max_value;
    std::size_t count;
};

struct MetadataSummary {
    std::size_t total_blocks;
    std::size_t total_elements;
};

std::vector<BlockMetadata> generate_metadata(
    const std::vector<std::int64_t>& data,
    const std::vector<Chunk>& chunks
);

bool can_contain_target(const BlockMetadata& metadata, std::int64_t target);

MetadataSummary summarize_metadata(const std::vector<BlockMetadata>& metadata);

} // namespace sage
