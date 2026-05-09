#include "chunk_manager.hpp"
#include <stdexcept>

namespace sage {

std::vector<Chunk> create_chunks(std::size_t dataset_size, std::size_t chunk_count) {
    if (dataset_size == 0) {
        return {};
    }
    
    if (chunk_count == 0) {
        chunk_count = 1;
    }
    
    // Don't create more chunks than elements
    chunk_count = std::min(chunk_count, dataset_size);
    
    std::vector<Chunk> chunks;
    chunks.reserve(chunk_count);
    
    const std::size_t base_chunk_size = dataset_size / chunk_count;
    const std::size_t remainder = dataset_size % chunk_count;
    
    std::size_t start_idx = 0;
    for (std::size_t i = 0; i < chunk_count; ++i) {
        std::size_t end_idx = start_idx + base_chunk_size;
        // Distribute remainder to early chunks
        if (i < remainder) {
            ++end_idx;
        }
        
        chunks.push_back({i, start_idx, end_idx});
        start_idx = end_idx;
    }
    
    return chunks;
}

bool validate_chunks(const std::vector<Chunk>& chunks, std::size_t dataset_size) {
    // Empty chunks are only valid when dataset_size == 0
    if (chunks.empty()) {
        return dataset_size == 0;
    }
    
    // First chunk must start at 0
    if (chunks[0].start != 0) {
        return false;
    }
    
    // Last chunk must end at dataset_size
    if (chunks.back().end != dataset_size) {
        return false;
    }
    
    // Check for proper ordering and no gaps/overlaps
    for (std::size_t i = 0; i < chunks.size(); ++i) {
        const auto& chunk = chunks[i];
        
        // Each chunk must have start < end
        if (chunk.start >= chunk.end) {
            return false;
        }
        
        // Check that each chunk start equals previous chunk end (except first)
        if (i > 0 && chunk.start != chunks[i-1].end) {
            return false;
        }
    }
    
    return true;
}

} // namespace sage
