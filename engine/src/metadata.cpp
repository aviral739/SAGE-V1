#include "metadata.hpp"
#include <algorithm>

namespace sage {

std::vector<BlockMetadata> generate_metadata(
    const std::vector<std::int64_t>& data,
    const std::vector<Chunk>& chunks
) {
    std::vector<BlockMetadata> metadata;
    metadata.reserve(chunks.size());
    
    for (const auto& chunk : chunks) {
        // Skip invalid chunks
        if (chunk.start >= chunk.end || chunk.end > data.size()) {
            continue;
        }
        
        std::int64_t min_val = data[chunk.start];
        std::int64_t max_val = data[chunk.start];
        
        // Scan chunk to find min and max
        for (std::size_t i = chunk.start; i < chunk.end; ++i) {
            min_val = std::min(min_val, data[i]);
            max_val = std::max(max_val, data[i]);
        }
        
        metadata.push_back({
            chunk.id,
            chunk.start,
            chunk.end,
            min_val,
            max_val,
            chunk.end - chunk.start
        });
    }
    
    return metadata;
}

bool can_contain_target(const BlockMetadata& metadata, std::int64_t target) {
    return target >= metadata.min_value && target <= metadata.max_value;
}

MetadataSummary summarize_metadata(const std::vector<BlockMetadata>& metadata) {
    MetadataSummary summary{0, 0};
    
    for (const auto& block : metadata) {
        summary.total_blocks++;
        summary.total_elements += block.count;
    }
    
    return summary;
}

} // namespace sage
