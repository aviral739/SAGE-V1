#pragma once

#include <cstddef>

namespace sage {

std::size_t logical_core_count();

std::size_t recommended_worker_count(std::size_t dataset_size);

} // namespace sage
