#pragma once

#include <vector>
#include <cstddef>

namespace sage {

struct Chunk {
    std::size_t id;
    std::size_t start;
    std::size_t end; // exclusive, meaning [start, end)
};

std::vector<Chunk> create_chunks(std::size_t dataset_size, std::size_t chunk_count);

bool validate_chunks(const std::vector<Chunk>& chunks, std::size_t dataset_size);

} // namespace sage
