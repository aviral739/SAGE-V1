#include "dataset_generator.hpp"
#include <random>
#include <algorithm>

namespace sage {

GeneratedDataset generate_dataset(std::size_t size, TargetPlacement placement) {
    GeneratedDataset result;
    result.placement = placement;
    result.target = 9'999'999'937; // Fixed target value for most cases
    
    // Handle empty dataset
    if (size == 0) {
        result.expected_index = std::nullopt;
        return result;
    }
    
    // Generate random unsorted data
    std::mt19937_64 rng(42); // Fixed seed for reproducibility
    std::uniform_int_distribution<std::int64_t> dist(1, 9'999'999'936); // Avoid target value
    
    result.data.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        result.data.push_back(dist(rng));
    }
    
    // Insert target based on placement
    std::size_t target_index = 0;
    
    switch (placement) {
        case TargetPlacement::BEGINNING:
            target_index = 0;
            break;
        case TargetPlacement::MIDDLE:
            target_index = size / 2;
            break;
        case TargetPlacement::END:
            target_index = size - 1;
            break;
        case TargetPlacement::RANDOM: {
            std::uniform_int_distribution<std::size_t> index_dist(0, size - 1);
            target_index = index_dist(rng);
            break;
        }
        case TargetPlacement::ABSENT:
            result.expected_index = std::nullopt;
            // Shuffle to ensure unsorted nature
            std::shuffle(result.data.begin(), result.data.end(), rng);
            return result;
        case TargetPlacement::ABSENT_IN_RANGE:
            // Use target value 500000 that's within normal range
            result.target = 500000;
            // Ensure random generation never accidentally generates 500000
            std::uniform_int_distribution<std::int64_t> absent_dist(1, 9'999'999'936);
            for (std::size_t i = 0; i < size; ++i) {
                std::int64_t value;
                do {
                    value = absent_dist(rng);
                } while (value == 500000); // Ensure we never generate the target
                result.data[i] = value;
            }
            result.expected_index = std::nullopt;
            // Shuffle to ensure unsorted nature
            std::shuffle(result.data.begin(), result.data.end(), rng);
            return result;
    }
    
    // Insert target at the calculated position
    result.data[target_index] = result.target;
    result.expected_index = target_index;
    
    // Shuffle remaining elements to maintain unsorted nature
    // but keep target at its intended position
    if (size > 1) {
        // Create a copy of data without target position
        std::vector<std::int64_t> other_data;
        other_data.reserve(size - 1);
        for (std::size_t i = 0; i < size; ++i) {
            if (i != target_index) {
                other_data.push_back(result.data[i]);
            }
        }
        
        // Shuffle the other elements
        std::shuffle(other_data.begin(), other_data.end(), rng);
        
        // Reconstruct data with target at correct position
        std::size_t other_idx = 0;
        for (std::size_t i = 0; i < size; ++i) {
            if (i != target_index) {
                result.data[i] = other_data[other_idx++];
            }
        }
    }
    
    return result;
}

} // namespace sage
