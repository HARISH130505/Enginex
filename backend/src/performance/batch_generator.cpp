#include "workbench/performance/batch_generator.hpp"
#include <random>

namespace enginex::performance {

nlohmann::json SimulationInput::to_json() const {
    return nlohmann::json{
        {"id", id},
        {"params", params.to_json()},
        {"N", N}
    };
}

std::vector<SimulationInput> BatchGenerator::generate(
    size_t count,
    uint32_t seed,
    size_t elements_per_sim,
    double k_min,
    double k_max,
    double q_min,
    double q_max)
{
    std::vector<SimulationInput> batch;
    batch.reserve(count);

    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist_k(k_min, k_max);
    std::uniform_real_distribution<double> dist_q(q_min, q_max);

    for (size_t i = 0; i < count; ++i) {
        SimulationInput sim;
        sim.id = i;
        sim.N = elements_per_sim;
        sim.params.L = 1.0;
        sim.params.A = 1.0e-4;
        sim.params.k = dist_k(rng);
        sim.params.q = dist_q(rng);
        sim.params.T_left = 100.0;
        sim.params.T_right = 20.0;
        batch.push_back(std::move(sim));
    }

    return batch;
}

} // namespace enginex::performance

