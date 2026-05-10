#include "csv_export.hpp"
#include <fstream>
#include <filesystem>
#include <iomanip>

namespace sage {

void write_csv_header_if_needed(const std::string& file_path) {
    // Create parent directories if needed
    std::filesystem::path path(file_path);
    std::filesystem::create_directories(path.parent_path());
    
    // Check if file exists or is empty
    std::ifstream file(file_path);
    if (!file.good() || file.peek() == std::ifstream::traits_type::eof()) {
        file.close();
        
        // Write header
        std::ofstream out(file_path);
        if (out.good()) {
            out << "version,placement,dataset_size,target_value,resource_mode,selected_strategy,"
                << "logical_cores,workers,total_blocks,blocks_searched,blocks_skipped,"
                << "skip_percentage,linear_ms,std_find_ms,parallel_ms,"
                << "metadata_pruned_ms,parallel_speedup,metadata_pruned_speedup,"
                << "linear_correct,std_find_correct,parallel_correct,metadata_pruned_correct\n";
        }
    }
}

void append_benchmark_row(const std::string& file_path, const BenchmarkCsvRow& row) {
    std::ofstream out(file_path, std::ios::app);
    if (!out.good()) {
        return;
    }
    
    out << std::fixed << std::setprecision(6);
    
    out << row.version << ","
        << row.placement << ","
        << row.dataset_size << ","
        << row.target_value << ","
        << row.resource_mode << ","
        << row.selected_strategy << ","
        << row.logical_cores << ","
        << row.workers << ","
        << row.total_blocks << ","
        << row.blocks_searched << ","
        << row.blocks_skipped << ","
        << row.skip_percentage << ","
        << row.linear_ms << ","
        << row.std_find_ms << ","
        << row.parallel_ms << ","
        << row.metadata_pruned_ms << ","
        << row.parallel_speedup << ","
        << row.metadata_pruned_speedup << ","
        << (row.linear_correct ? "YES" : "NO") << ","
        << (row.std_find_correct ? "YES" : "NO") << ","
        << (row.parallel_correct ? "YES" : "NO") << ","
        << (row.metadata_pruned_correct ? "YES" : "NO") << std::endl;
}

} // namespace sage
