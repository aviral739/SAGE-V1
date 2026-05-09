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

} // namespace sage
