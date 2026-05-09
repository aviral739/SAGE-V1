#include "strategy.hpp"
#include "resource_policy.hpp"

namespace sage {

StrategyDecision choose_strategy(
    std::size_t dataset_size,
    bool metadata_available,
    ResourceMode mode
) {
    StrategyDecision decision;
    decision.workers = recommended_worker_count(dataset_size, mode);
    
    if (dataset_size < 1'000'000) {
        decision.strategy = SearchStrategy::LINEAR;
        decision.reason = "Small dataset (< 1M elements) - linear search is most efficient";
    } else if (metadata_available) {
        decision.strategy = SearchStrategy::METADATA_PRUNED_PARALLEL;
        decision.reason = "Large dataset with metadata available - using metadata-pruned parallel search";
    } else {
        decision.strategy = SearchStrategy::PARALLEL;
        decision.reason = "Large dataset without metadata - using standard parallel search";
    }
    
    return decision;
}

const char* to_string(SearchStrategy strategy) {
    switch (strategy) {
        case SearchStrategy::LINEAR: return "LINEAR";
        case SearchStrategy::STD_FIND: return "STD_FIND";
        case SearchStrategy::PARALLEL: return "PARALLEL";
        case SearchStrategy::METADATA_PRUNED_PARALLEL: return "METADATA_PRUNED_PARALLEL";
        default: return "UNKNOWN";
    }
}

} // namespace sage
