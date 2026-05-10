#include "search.hpp"
#include <algorithm>
#include <thread>
#include <atomic>
#include <vector>

namespace sage {

std::optional<std::size_t> linear_search(
    const std::vector<std::int64_t>& data,
    std::int64_t target
) {
    if (data.empty()) {
        return std::nullopt;
    }
    
    for (std::size_t i = 0; i < data.size(); ++i) {
        if (data[i] == target) {
            return i;
        }
    }
    
    return std::nullopt;
}

std::optional<std::size_t> std_find_search(
    const std::vector<std::int64_t>& data,
    std::int64_t target
) {
    if (data.empty()) {
        return std::nullopt;
    }
    
    auto it = std::find(data.begin(), data.end(), target);
    if (it != data.end()) {
        return static_cast<std::size_t>(it - data.begin());
    }
    
    return std::nullopt;
}

namespace {
    void search_chunk(
        const std::vector<std::int64_t>& data,
        std::int64_t target,
        std::size_t start_idx,
        std::size_t end_idx,
        std::atomic<bool>& found,
        std::atomic<std::size_t>& result_idx
    ) {
        for (std::size_t i = start_idx; i < end_idx && !found.load(); ++i) {
            if (data[i] == target) {
                // Try to claim the find
                bool expected = false;
                if (found.compare_exchange_strong(expected, true)) {
                    result_idx.store(i);
                    return;
                }
            }
        }
    }
}

std::optional<std::size_t> parallel_search(
    const std::vector<std::int64_t>& data,
    std::int64_t target,
    std::size_t worker_count
) {
    if (data.empty()) {
        return std::nullopt;
    }
    
    // Ensure at least 1 worker
    if (worker_count == 0) {
        worker_count = 1;
    }
    
    // Don't create more workers than data elements
    worker_count = std::min(worker_count, data.size());
    
    std::atomic<bool> found(false);
    std::atomic<std::size_t> result_idx(0);
    std::vector<std::thread> workers;
    
    // Calculate chunk size
    const std::size_t chunk_size = data.size() / worker_count;
    const std::size_t remainder = data.size() % worker_count;
    
    std::size_t start_idx = 0;
    for (std::size_t i = 0; i < worker_count; ++i) {
        std::size_t end_idx = start_idx + chunk_size;
        // Distribute remainder to early chunks
        if (i < remainder) {
            ++end_idx;
        }
        
        workers.emplace_back(search_chunk, std::ref(data), target, 
                            start_idx, end_idx, std::ref(found), std::ref(result_idx));
        
        start_idx = end_idx;
    }
    
    // Wait for all workers to complete
    for (auto& worker : workers) {
        worker.join();
    }
    
    if (found.load()) {
        return result_idx.load();
    }
    
    return std::nullopt;
}

namespace {
    struct SearchRange {
        std::size_t start;
        std::size_t end;
    };
    
    void search_ranges(
        const std::vector<std::int64_t>& data,
        std::int64_t target,
        const std::vector<SearchRange>& ranges,
        std::size_t start_idx,
        std::size_t end_idx,
        std::atomic<bool>& found,
        std::atomic<std::size_t>& result_idx
    ) {
        for (std::size_t i = start_idx; i < end_idx && !found.load(); ++i) {
            const auto& range = ranges[i];
            
            // Safety check
            if (range.start >= range.end || range.end > data.size()) {
                continue;
            }
            
            // Search within this range
            for (std::size_t j = range.start; j < range.end && !found.load(); ++j) {
                if (data[j] == target) {
                    // Try to claim the find
                    bool expected = false;
                    if (found.compare_exchange_strong(expected, true)) {
                        result_idx.store(j);
                        return;
                    }
                }
            }
        }
    }
}

MetadataSearchResult metadata_pruned_parallel_search(
    const std::vector<std::int64_t>& data,
    std::int64_t target,
    const std::vector<BlockMetadata>& metadata,
    std::size_t worker_count
) {
    MetadataSearchResult result;
    result.result_index = std::nullopt;
    result.total_blocks = metadata.size();
    result.blocks_searched = 0;
    result.blocks_skipped = 0;
    
    // Handle edge cases
    if (data.empty() || metadata.empty()) {
        return result;
    }
    
    if (worker_count == 0) {
        worker_count = 1;
    }
    
    // Metadata pruning - find eligible blocks
    std::vector<BlockMetadata> eligible_blocks;
    eligible_blocks.reserve(metadata.size());
    
    for (const auto& block : metadata) {
        if (sage::can_contain_target(block, target)) {
            eligible_blocks.push_back(block);
        }
    }
    
    result.blocks_skipped = result.total_blocks - eligible_blocks.size();
    result.blocks_searched = eligible_blocks.size();
    
    // If no eligible blocks, return not found
    if (eligible_blocks.empty()) {
        return result;
    }
    
    // Create search ranges from eligible blocks
    std::vector<SearchRange> ranges;
    
    if (eligible_blocks.size() >= worker_count) {
        // More blocks than workers - assign blocks to workers
        for (const auto& block : eligible_blocks) {
            ranges.push_back({block.start, block.end});
        }
    } else {
        // Fewer blocks than workers - split large blocks into ranges
        std::size_t total_elements = 0;
        for (const auto& block : eligible_blocks) {
            total_elements += (block.end - block.start);
        }
        
        std::size_t target_range_size = total_elements / worker_count;
        std::size_t current_pos = 0;
        std::size_t current_block_idx = 0;
        
        for (std::size_t i = 0; i < worker_count && current_block_idx < eligible_blocks.size(); ++i) {
            std::size_t range_start = current_pos;
            std::size_t range_end = current_pos;
            std::size_t elements_in_range = 0;
            
            // Start from current position, which should be at block.start
            while (elements_in_range < target_range_size && current_block_idx < eligible_blocks.size()) {
                const auto& block = eligible_blocks[current_block_idx];
                
                // If we haven't started this block yet, position should be at block.start
                if (current_pos < block.start) {
                    current_pos = block.start;
                    range_start = current_pos;
                }
                
                // Calculate how much we can take from this block
                std::size_t needed = target_range_size - elements_in_range;
                std::size_t available_in_block = block.end - current_pos;
                std::size_t to_take = std::min(needed, available_in_block);
                
                range_end = current_pos + to_take;
                elements_in_range += to_take;
                current_pos = range_end;
                
                // If we've consumed this entire block, move to next
                if (current_pos >= block.end) {
                    current_block_idx++;
                }
            }
            
            // Ensure we don't create empty ranges
            if (range_start < range_end) {
                ranges.push_back({range_start, range_end});
            }
        }
    }
    
    // Don't create more workers than ranges
    worker_count = std::min(worker_count, ranges.size());
    
    std::atomic<bool> found(false);
    std::atomic<std::size_t> result_idx(0);
    std::vector<std::thread> workers;
    
    // Calculate range distribution
    const std::size_t ranges_per_worker = ranges.size() / worker_count;
    const std::size_t remainder = ranges.size() % worker_count;
    
    std::size_t start_range = 0;
    for (std::size_t i = 0; i < worker_count; ++i) {
        std::size_t end_range = start_range + ranges_per_worker;
        // Distribute remainder to early workers
        if (i < remainder) {
            ++end_range;
        }
        
        workers.emplace_back(search_ranges, std::ref(data), target, 
                            std::ref(ranges), start_range, end_range, 
                            std::ref(found), std::ref(result_idx));
        
        start_range = end_range;
    }
    
    // Wait for all workers to complete
    for (auto& worker : workers) {
        worker.join();
    }
    
    if (found.load()) {
        result.result_index = result_idx.load();
    }
    
    return result;
}

} // namespace sage
