#pragma once

#include <vector>
#include <cstdint>
#include <optional>

namespace sage {

enum class TargetPlacement {
    BEGINNING,
    MIDDLE,
    END,
    RANDOM,
    ABSENT,
    ABSENT_IN_RANGE
};

struct GeneratedDataset {
    std::vector<std::int64_t> data;
    std::int64_t target;
    std::optional<std::size_t> expected_index;
    TargetPlacement placement;
};

GeneratedDataset generate_dataset(std::size_t size, TargetPlacement placement);

} // namespace sage
