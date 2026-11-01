#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include "workbench/thermal/solver_1d.hpp"
#include "workbench/core/json.hpp"

namespace enginex::performance {

// A single thermal simulation job within a batch
struct SimulationInput {
    size_t id = 0;
    thermal::RodParameters params;
    size_t N = 40;

    nlohmann::json to_json() const;
};

class BatchGenerator {
public:
    // Generate a reproducible, deterministic batch of simulation inputs
    // k is distributed in [k_min, k_max] W/(m·K)
    // q is distributed in [q_min, q_max] W/m^3
    static std::vector<SimulationInput> generate(
        size_t count,
        uint32_t seed = 42,
        size_t elements_per_sim = 40,
        double k_min = 1.0,
        double k_max = 100.0,
        double q_min = 1000.0,
        double q_max = 50000.0
    );
};

} // namespace enginex::performance

