#pragma once

#include "resource_policy.hpp"
#include <string>
#include <cstddef>

namespace sage {

enum class SearchStrategy {
    LINEAR,
    STD_FIND,
    PARALLEL,
    METADATA_PRUNED_PARALLEL
};

struct StrategyDecision {
    SearchStrategy strategy;
    std::string reason;
    std::size_t workers;
};

StrategyDecision choose_strategy(
    std::size_t dataset_size,
    bool metadata_available,
    ResourceMode mode
);

const char* to_string(SearchStrategy strategy);

} // namespace sage
