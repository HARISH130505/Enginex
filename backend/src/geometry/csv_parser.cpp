#include "workbench/geometry/csv_parser.hpp"
#include <sstream>
#include <cmath>
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace enginex::geometry {

nlohmann::json TemperatureNode::to_json() const {
    return nlohmann::json{
        {"node_id", node_id},
        {"x_m", x_m},
        {"y_m", y_m},
        {"z_m", z_m},
        {"temperature_C", temperature_C}
    };
}

nlohmann::json TemperatureDataSet::to_json(bool include_nodes, size_t max_nodes_export) const {
    nlohmann::json j;
    j["filename"] = filename;
    j["node_count"] = nodes.size();
    j["bounding_box"] = bbox.to_json();
    j["temp_min"] = temp_min;
    j["temp_max"] = temp_max;
    j["temp_mean"] = temp_mean;
    j["temp_units"] = "°C";
    j["coordinate_units"] = "m";
    j["provenance"] = provenance;
    j["duplicate_node_count"] = duplicate_node_count;

    if (include_nodes) {
        nlohmann::json node_list = nlohmann::json::array();
        size_t count = std::min(nodes.size(), max_nodes_export);
        for (size_t i = 0; i < count; ++i) {
            node_list.push_back(nodes[i].to_json());
        }
        j["nodes"] = node_list;
        j["nodes_truncated"] = (nodes.size() > max_nodes_export);
    }

    return j;
}

namespace {
    std::string trim_csv(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n\"");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n\"");
        return str.substr(first, (last - first + 1));
    }

    std::vector<std::string> split_csv_line(const std::string& line) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ',')) {
            tokens.push_back(trim_csv(token));
        }
        return tokens;
    }
}

core::Result<TemperatureDataSet> CsvParser::parse_ansys_csv(
    std::istream& is,
    const std::string& filename,
    DuplicatePolicy dup_policy,
    size_t max_rows
) {
    std::string header_line;
    if (!std::getline(is, header_line)) {
        return core::Result<TemperatureDataSet>::fail(
            core::StatusCode::InvalidArgument,
            "Failed to parse CSV: File is completely empty."
        );
    }

    auto headers = split_csv_line(header_line);
    if (headers.empty()) {
        return core::Result<TemperatureDataSet>::fail(
            core::StatusCode::InvalidArgument,
            "Failed to parse CSV: Header row is empty."
        );
    }

    // Map required column names to their indices
    // Required schema: node_id,x_m,y_m,z_m,temperature_C
    int col_node_id = -1;
    int col_x_m = -1;
    int col_y_m = -1;
    int col_z_m = -1;
    int col_temp = -1;

    for (size_t i = 0; i < headers.size(); ++i) {
        std::string h = headers[i];
        std::transform(h.begin(), h.end(), h.begin(), [](unsigned char c) { return std::tolower(c); });

        if (h == "node_id" || h == "node" || h == "id") col_node_id = static_cast<int>(i);
        else if (h == "x_m" || h == "x") col_x_m = static_cast<int>(i);
        else if (h == "y_m" || h == "y") col_y_m = static_cast<int>(i);
        else if (h == "z_m" || h == "z") col_z_m = static_cast<int>(i);
        else if (h == "temperature_c" || h == "temperature" || h == "temp_c") col_temp = static_cast<int>(i);
    }

    // Check for missing required columns
    std::string missing_cols;
    if (col_node_id == -1) missing_cols += "node_id ";
    if (col_x_m == -1) missing_cols += "x_m ";
    if (col_y_m == -1) missing_cols += "y_m ";
    if (col_z_m == -1) missing_cols += "z_m ";
    if (col_temp == -1) missing_cols += "temperature_C ";

    if (!missing_cols.empty()) {
        return core::Result<TemperatureDataSet>::fail(
            core::StatusCode::InvalidArgument,
            "Missing required column(s) in CSV header: " + missing_cols +
            "(Expected exact schema: node_id,x_m,y_m,z_m,temperature_C)"
        );
    }

    TemperatureDataSet dataset;
    dataset.filename = filename;
    dataset.temp_min = std::numeric_limits<double>::infinity();
    dataset.temp_max = -std::numeric_limits<double>::infinity();

    std::unordered_map<int64_t, size_t> seen_node_ids;
    std::string line;
    size_t row_number = 1;
    double temp_sum = 0.0;

    while (std::getline(is, line)) {
        ++row_number;
        std::string trimmed = trim_csv(line);
        if (trimmed.empty()) continue;

        auto tokens = split_csv_line(line);
        if (tokens.size() <= static_cast<size_t>(std::max({col_node_id, col_x_m, col_y_m, col_z_m, col_temp}))) {
            return core::Result<TemperatureDataSet>::fail(
                core::StatusCode::InvalidArgument,
                "Malformed row at line " + std::to_string(row_number) + ": Expected at least " +
                std::to_string(headers.size()) + " columns, found " + std::to_string(tokens.size()) + "."
            );
        }

        // Parse node_id
        int64_t node_id = 0;
        try {
            size_t idx = 0;
            node_id = std::stoll(tokens[col_node_id], &idx);
            if (idx != tokens[col_node_id].size()) throw std::invalid_argument("trailing characters");
        } catch (...) {
            return core::Result<TemperatureDataSet>::fail(
                core::StatusCode::InvalidArgument,
                "Invalid non-integer node_id '" + tokens[col_node_id] + "' on line " + std::to_string(row_number) + "."
            );
        }

        // Parse coordinates and temperature
        double x_m = 0.0, y_m = 0.0, z_m = 0.0, temp = 0.0;
        try {
            size_t idx = 0;
            x_m = std::stod(tokens[col_x_m], &idx);
            if (idx != tokens[col_x_m].size()) throw std::invalid_argument("bad x");

            idx = 0;
            y_m = std::stod(tokens[col_y_m], &idx);
            if (idx != tokens[col_y_m].size()) throw std::invalid_argument("bad y");

            idx = 0;
            z_m = std::stod(tokens[col_z_m], &idx);
            if (idx != tokens[col_z_m].size()) throw std::invalid_argument("bad z");

            idx = 0;
            temp = std::stod(tokens[col_temp], &idx);
            if (idx != tokens[col_temp].size()) throw std::invalid_argument("bad temp");
        } catch (...) {
            return core::Result<TemperatureDataSet>::fail(
                core::StatusCode::InvalidArgument,
                "Malformed numeric floating-point value on line " + std::to_string(row_number) + "."
            );
        }

        // Check finite
        if (!std::isfinite(x_m) || !std::isfinite(y_m) || !std::isfinite(z_m) || !std::isfinite(temp)) {
            return core::Result<TemperatureDataSet>::fail(
                core::StatusCode::InvalidArgument,
                "Non-finite value (NaN or Infinity) detected on line " + std::to_string(row_number) + "."
            );
        }

        // Check duplicate policy
        auto existing = seen_node_ids.find(node_id);
        if (existing != seen_node_ids.end()) {
            if (dup_policy == DuplicatePolicy::RejectOnDuplicate) {
                return core::Result<TemperatureDataSet>::fail(
                    core::StatusCode::InvalidArgument,
                    "Duplicate node_id " + std::to_string(node_id) + " encountered on line " +
                    std::to_string(row_number) + " (Policy: RejectOnDuplicate)."
                );
            } else if (dup_policy == DuplicatePolicy::KeepFirst) {
                ++dataset.duplicate_node_count;
                continue; // Skip inserting duplicate
            } else if (dup_policy == DuplicatePolicy::Overwrite) {
                ++dataset.duplicate_node_count;
                dataset.nodes[existing->second] = {node_id, x_m, y_m, z_m, temp};
                continue;
            }
        }

        seen_node_ids[node_id] = dataset.nodes.size();
        dataset.nodes.push_back({node_id, x_m, y_m, z_m, temp});
        dataset.bbox.expand({x_m, y_m, z_m});

        dataset.temp_min = std::min(dataset.temp_min, temp);
        dataset.temp_max = std::max(dataset.temp_max, temp);
        temp_sum += temp;

        if (dataset.nodes.size() > max_rows) {
            return core::Result<TemperatureDataSet>::fail(
                core::StatusCode::InvalidArgument,
                "Row count exceeds configured maximum limit of " + std::to_string(max_rows) + " rows."
            );
        }
    }

    if (dataset.nodes.empty()) {
        return core::Result<TemperatureDataSet>::fail(
            core::StatusCode::InvalidArgument,
            "Empty dataset: CSV contains a header but zero data rows."
        );
    }

    dataset.temp_mean = temp_sum / static_cast<double>(dataset.nodes.size());
    return core::Result<TemperatureDataSet>::ok(std::move(dataset));
}

core::Result<TemperatureDataSet> CsvParser::parse_ansys_csv_string(
    const std::string& content,
    const std::string& filename,
    DuplicatePolicy dup_policy,
    size_t max_rows
) {
    std::istringstream iss(content);
    return parse_ansys_csv(iss, filename, dup_policy, max_rows);
}

} // namespace enginex::geometry

