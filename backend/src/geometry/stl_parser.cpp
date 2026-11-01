#include "workbench/geometry/stl_parser.hpp"
#include <sstream>
#include <cmath>
#include <cctype>
#include <algorithm>

namespace enginex::geometry {

bool Vector3D::is_finite() const noexcept {
    return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
}

nlohmann::json Vector3D::to_json() const {
    return nlohmann::json{{"x", x}, {"y", y}, {"z", z}};
}

nlohmann::json Triangle::to_json() const {
    return nlohmann::json{
        {"normal", normal.to_json()},
        {"v1", v1.to_json()},
        {"v2", v2.to_json()},
        {"v3", v3.to_json()}
    };
}

void BoundingBox::expand(const Vector3D& pt) {
    min.x = std::min(min.x, pt.x);
    min.y = std::min(min.y, pt.y);
    min.z = std::min(min.z, pt.z);

    max.x = std::max(max.x, pt.x);
    max.y = std::max(max.y, pt.y);
    max.z = std::max(max.z, pt.z);
}

Vector3D BoundingBox::dimensions() const {
    return Vector3D{max.x - min.x, max.y - min.y, max.z - min.z};
}

Vector3D BoundingBox::center() const {
    return Vector3D{
        (min.x + max.x) * 0.5,
        (min.y + max.y) * 0.5,
        (min.z + max.z) * 0.5
    };
}

nlohmann::json BoundingBox::to_json() const {
    auto dims = dimensions();
    auto ctr = center();
    return nlohmann::json{
        {"min", min.to_json()},
        {"max", max.to_json()},
        {"dimensions", dims.to_json()},
        {"center", ctr.to_json()}
    };
}

nlohmann::json StlMesh::to_json(bool include_triangles, size_t max_triangles_export) const {
    nlohmann::json j;
    j["name"] = name;
    j["filename"] = filename;
    j["triangle_count"] = triangles.size();
    j["bounding_box"] = bbox.to_json();
    j["file_bytes"] = file_bytes;
    j["format"] = "ASCII STL";
    j["units_assumption"] = "Dataset assumption (STL does not encode physical units)";

    if (include_triangles) {
        nlohmann::json tri_list = nlohmann::json::array();
        size_t count = std::min(triangles.size(), max_triangles_export);
        for (size_t i = 0; i < count; ++i) {
            tri_list.push_back(triangles[i].to_json());
        }
        j["triangles"] = tri_list;
        j["triangles_truncated"] = (triangles.size() > max_triangles_export);
    }

    return j;
}

namespace {
    std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    std::string to_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return str;
    }
}

core::Result<StlMesh> StlParser::parse_ascii(
    std::istream& is,
    const std::string& filename,
    size_t max_bytes,
    size_t max_triangles
) {
    std::string line;
    size_t line_number = 0;
    size_t total_bytes = 0;

    // Check empty
    if (!std::getline(is, line)) {
        return core::Result<StlMesh>::fail(
            core::StatusCode::InvalidArgument,
            "Failed to parse STL: File is completely empty."
        );
    }
    ++line_number;
    total_bytes += line.size() + 1;

    std::string trimmed_first = trim(line);
    std::string lower_first = to_lower(trimmed_first);

    if (lower_first.rfind("solid", 0) != 0) {
        return core::Result<StlMesh>::fail(
            core::StatusCode::InvalidArgument,
            "Invalid ASCII STL format: First line must begin with 'solid' keyword. Found: '" + trimmed_first.substr(0, 30) + "'"
        );
    }

    StlMesh mesh;
    mesh.filename = filename;
    if (trimmed_first.size() > 5) {
        mesh.name = trim(trimmed_first.substr(5));
    }
    if (mesh.name.empty()) {
        mesh.name = "unnamed_solid";
    }

    Triangle current_triangle;
    int vertex_count = 0;
    bool inside_facet = false;
    bool inside_loop = false;

    while (std::getline(is, line)) {
        ++line_number;
        total_bytes += line.size() + 1;

        if (total_bytes > max_bytes) {
            return core::Result<StlMesh>::fail(
                core::StatusCode::InvalidArgument,
                "STL file size exceeds configured safety limit of " + std::to_string(max_bytes) + " bytes."
            );
        }

        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;

        std::string lower = to_lower(trimmed);

        if (lower.rfind("facet", 0) == 0) {
            if (inside_facet) {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Malformed STL syntax at line " + std::to_string(line_number) + ": Nested 'facet' without closing preceding facet."
                );
            }
            inside_facet = true;
            inside_loop = false;
            vertex_count = 0;

            // Parse facet normal
            std::stringstream ss(trimmed);
            std::string facet_word, normal_word;
            double nx = 0.0, ny = 0.0, nz = 0.0;
            ss >> facet_word >> normal_word >> nx >> ny >> nz;

            if (ss.fail() || !std::isfinite(nx) || !std::isfinite(ny) || !std::isfinite(nz)) {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Malformed normal coordinates at line " + std::to_string(line_number) + ": Expected 3 finite floats."
                );
            }
            current_triangle.normal = {nx, ny, nz};

        } else if (lower.rfind("outer loop", 0) == 0) {
            if (!inside_facet) {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Malformed STL syntax at line " + std::to_string(line_number) + ": 'outer loop' found outside of facet."
                );
            }
            inside_loop = true;
            vertex_count = 0;

        } else if (lower.rfind("vertex", 0) == 0) {
            if (!inside_loop) {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Malformed STL syntax at line " + std::to_string(line_number) + ": 'vertex' found outside of outer loop."
                );
            }

            std::stringstream ss(trimmed);
            std::string vertex_word;
            double vx = 0.0, vy = 0.0, vz = 0.0;
            ss >> vertex_word >> vx >> vy >> vz;

            if (ss.fail() || !std::isfinite(vx) || !std::isfinite(vy) || !std::isfinite(vz)) {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Invalid or non-finite vertex coordinates (NaN/Inf) on line " + std::to_string(line_number) + "."
                );
            }

            Vector3D v{vx, vy, vz};
            if (vertex_count == 0) current_triangle.v1 = v;
            else if (vertex_count == 1) current_triangle.v2 = v;
            else if (vertex_count == 2) current_triangle.v3 = v;
            else {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Invalid vertex count on line " + std::to_string(line_number) + ": Facet loop contains more than 3 vertices."
                );
            }
            ++vertex_count;
            mesh.bbox.expand(v);

        } else if (lower.rfind("endloop", 0) == 0) {
            if (!inside_loop || vertex_count != 3) {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Truncated STL facet on line " + std::to_string(line_number) + ": Expected exactly 3 vertices in loop, found " + std::to_string(vertex_count) + "."
                );
            }
            inside_loop = false;

        } else if (lower.rfind("endfacet", 0) == 0) {
            if (!inside_facet || inside_loop || vertex_count != 3) {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Malformed STL syntax at line " + std::to_string(line_number) + ": 'endfacet' encountered before completing outer loop."
                );
            }
            inside_facet = false;
            mesh.triangles.push_back(current_triangle);

            if (mesh.triangles.size() > max_triangles) {
                return core::Result<StlMesh>::fail(
                    core::StatusCode::InvalidArgument,
                    "Triangle count exceeds configured limit of " + std::to_string(max_triangles) + "."
                );
            }

        } else if (lower.rfind("endsolid", 0) == 0) {
            break;
        }
    }

    if (inside_facet || inside_loop) {
        return core::Result<StlMesh>::fail(
            core::StatusCode::InvalidArgument,
            "Truncated STL file: File ended prematurely while parsing a facet."
        );
    }

    if (mesh.triangles.empty()) {
        return core::Result<StlMesh>::fail(
            core::StatusCode::InvalidArgument,
            "Empty geometry: File contains no valid triangles."
        );
    }

    mesh.file_bytes = total_bytes;
    return core::Result<StlMesh>::ok(std::move(mesh));
}

core::Result<StlMesh> StlParser::parse_ascii_string(
    const std::string& content,
    const std::string& filename,
    size_t max_triangles
) {
    std::istringstream iss(content);
    return parse_ascii(iss, filename, content.size() + 1024, max_triangles);
}

} // namespace enginex::geometry

