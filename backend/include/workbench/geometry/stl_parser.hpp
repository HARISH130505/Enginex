#pragma once

#include <string>
#include <vector>
#include <array>
#include <istream>
#include <limits>
#include "workbench/core/result.hpp"
#include "workbench/core/json.hpp"

namespace enginex::geometry {

struct Vector3D {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    bool is_finite() const noexcept;
    nlohmann::json to_json() const;
};

struct Triangle {
    Vector3D normal;
    Vector3D v1;
    Vector3D v2;
    Vector3D v3;

    nlohmann::json to_json() const;
};

struct BoundingBox {
    Vector3D min{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    Vector3D max{-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};

    void expand(const Vector3D& pt);
    Vector3D dimensions() const;
    Vector3D center() const;
    nlohmann::json to_json() const;
};

struct StlMesh {
    std::string name;
    std::string filename;
    std::vector<Triangle> triangles;
    BoundingBox bbox;
    size_t file_bytes{0};

    nlohmann::json to_json(bool include_triangles = true, size_t max_triangles_export = 20000) const;
};

class StlParser {
public:
    static constexpr size_t DEFAULT_MAX_FILE_SIZE = 50 * 1024 * 1024; // 50 MB limit
    static constexpr size_t DEFAULT_MAX_TRIANGLES = 500000;           // 500k triangles

    // Parse ASCII STL from stream
    static core::Result<StlMesh> parse_ascii(
        std::istream& is,
        const std::string& filename = "geometry.stl",
        size_t max_bytes = DEFAULT_MAX_FILE_SIZE,
        size_t max_triangles = DEFAULT_MAX_TRIANGLES
    );

    // Parse ASCII STL from raw text string
    static core::Result<StlMesh> parse_ascii_string(
        const std::string& content,
        const std::string& filename = "geometry.stl",
        size_t max_triangles = DEFAULT_MAX_TRIANGLES
    );
};

} // namespace enginex::geometry

