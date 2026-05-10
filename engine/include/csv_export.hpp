#pragma once

#include <string>
#include <cstddef>
#include <cstdint>

namespace sage {

struct BenchmarkCsvRow {
    std::string version;
    std::string placement;
    std::size_t dataset_size;
    std::int64_t target_value;
    std::string resource_mode;
    std::string selected_strategy;
    std::size_t logical_cores;
    std::size_t workers;
    std::size_t total_blocks;
    std::size_t blocks_searched;
    std::size_t blocks_skipped;
    double skip_percentage;
    double linear_ms;
    double std_find_ms;
    double parallel_ms;
    double metadata_pruned_ms;
    double parallel_speedup;
    double metadata_pruned_speedup;
    bool linear_correct;
    bool std_find_correct;
    bool parallel_correct;
    bool metadata_pruned_correct;
};

void write_csv_header_if_needed(const std::string& file_path);
void append_benchmark_row(const std::string& file_path, const BenchmarkCsvRow& row);

} // namespace sage
