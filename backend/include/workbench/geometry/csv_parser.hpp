#pragma once

#include <string>
#include <vector>
#include <istream>
#include <cstdint>
#include "workbench/core/result.hpp"
#include "workbench/geometry/stl_parser.hpp" // For Vector3D and BoundingBox
#include "workbench/core/json.hpp"

namespace enginex::geometry {

struct TemperatureNode {
    int64_t node_id{0};
    double x_m{0.0};
    double y_m{0.0};
    double z_m{0.0};
    double temperature_C{0.0};

    nlohmann::json to_json() const;
};

struct TemperatureDataSet {
    std::string filename;
    std::vector<TemperatureNode> nodes;
    BoundingBox bbox;
    double temp_min{0.0};
    double temp_max{0.0};
    double temp_mean{0.0};
    size_t duplicate_node_count{0};
    std::string provenance{"Synthetic CSV (Explicitly labeled; not physical ANSYS export)"};

    nlohmann::json to_json(bool include_nodes = true, size_t max_nodes_export = 50000) const;
};

enum class DuplicatePolicy {
    RejectOnDuplicate = 0, // Fail parse with actionable error
    KeepFirst,             // Retain first occurrence and increment count
    Overwrite              // Overwrite with newest occurrence
};

class CsvParser {
public:
    static constexpr size_t DEFAULT_MAX_ROWS = 500000;

    // Parse ANSYS nodal temperature CSV
    static core::Result<TemperatureDataSet> parse_ansys_csv(
        std::istream& is,
        const std::string& filename = "ansys_export.csv",
        DuplicatePolicy dup_policy = DuplicatePolicy::RejectOnDuplicate,
        size_t max_rows = DEFAULT_MAX_ROWS
    );

    // Parse from raw string
    static core::Result<TemperatureDataSet> parse_ansys_csv_string(
        const std::string& content,
        const std::string& filename = "ansys_export.csv",
        DuplicatePolicy dup_policy = DuplicatePolicy::RejectOnDuplicate,
        size_t max_rows = DEFAULT_MAX_ROWS
    );
};

} // namespace enginex::geometry

