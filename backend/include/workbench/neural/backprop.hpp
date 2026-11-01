#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include "workbench/core/result.hpp"
#include "workbench/core/json.hpp"

namespace enginex::neural {

struct ModelParameters {
    double w = 0.5;  // Input weight
    double b = 0.0;  // Hidden bias
    double v = 1.0;  // Output weight
    double c = 0.0;  // Output bias

    nlohmann::json to_json() const;
};

struct Gradients {
    double dJ_dw = 0.0;
    double dJ_db = 0.0;
    double dJ_dv = 0.0;
    double dJ_dc = 0.0;

    nlohmann::json to_json() const;
};

struct ForwardState {
    double x = 1.0;
    double y = 2.0;
    double z = 0.0;     // w*x + b
    double h = 0.0;     // tanh(z)
    double y_hat = 0.0; // v*h + c
    double loss = 0.0;  // 0.5*(y_hat - y)^2

    nlohmann::json to_json() const;
};

struct GradientCheckEntry {
    std::string param_name;
    double analytical = 0.0;
    double numerical = 0.0;
    double absolute_error = 0.0;
    bool passed = false;  // error <= 1e-5

    nlohmann::json to_json() const;
};

struct GradientCheckResult {
    double epsilon = 1.0e-6;
    double tolerance = 1.0e-5;
    std::vector<GradientCheckEntry> entries;
    bool all_passed = false;

    nlohmann::json to_json() const;
};

struct TrainingStep {
    size_t step = 0;
    double loss = 0.0;
    double y_hat = 0.0;
    ModelParameters params;
    Gradients grads;

    nlohmann::json to_json() const;
};

struct TrainingResult {
    ModelParameters initial_params;
    ModelParameters final_params;
    double learning_rate = 0.1;
    size_t total_steps = 100;
    GradientCheckResult grad_check;
    std::vector<TrainingStep> history;
    std::string csv_export;

    nlohmann::json to_json() const;
};

class BackpropCore {
public:
    // Forward pass: compute z, h, y_hat, loss for given parameters
    static ForwardState forward(double x, double y, const ModelParameters& p);

    // Compute exact analytical gradients via manual chain rule
    static Gradients backward(const ForwardState& state, const ModelParameters& p);

    // Compute numerical gradients using central finite differences:
    // [dJ/dtheta]_num = (J(theta + eps) - J(theta - eps)) / (2*eps)
    static Gradients numerical_gradients(
        double x, double y, const ModelParameters& p, double epsilon = 1.0e-6);

    // Validate analytical vs numerical gradients (tolerance <= 1e-5)
    static GradientCheckResult verify_gradients(
        double x, double y, const ModelParameters& p, double epsilon = 1.0e-6, double tol = 1.0e-5);

    // Execute full 100-step gradient descent training loop
    static core::Result<TrainingResult> train(
        double x = 1.0,
        double y = 2.0,
        ModelParameters initial_params = ModelParameters{},
        double lr = 0.1,
        size_t steps = 100,
        double epsilon = 1.0e-6
    );
};

} // namespace enginex::neural
