#include "workbench/neural/backprop.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace enginex::neural {

// ── JSON Serialization ──────────────────────────────────────────────────────

nlohmann::json ModelParameters::to_json() const {
    return nlohmann::json{
        {"w", w},
        {"b", b},
        {"v", v},
        {"c", c}
    };
}

nlohmann::json Gradients::to_json() const {
    return nlohmann::json{
        {"dJ_dw", dJ_dw},
        {"dJ_db", dJ_db},
        {"dJ_dv", dJ_dv},
        {"dJ_dc", dJ_dc}
    };
}

nlohmann::json ForwardState::to_json() const {
    return nlohmann::json{
        {"x", x},
        {"y", y},
        {"z", z},
        {"h", h},
        {"y_hat", y_hat},
        {"loss", loss}
    };
}

nlohmann::json GradientCheckEntry::to_json() const {
    return nlohmann::json{
        {"param_name", param_name},
        {"analytical", analytical},
        {"numerical", numerical},
        {"absolute_error", absolute_error},
        {"passed", passed}
    };
}

nlohmann::json GradientCheckResult::to_json() const {
    nlohmann::json j;
    j["epsilon"] = epsilon;
    j["tolerance"] = tolerance;
    j["all_passed"] = all_passed;

    nlohmann::json entries_json = nlohmann::json::array();
    for (const auto& e : entries) {
        entries_json.push_back(e.to_json());
    }
    j["entries"] = entries_json;
    return j;
}

nlohmann::json TrainingStep::to_json() const {
    return nlohmann::json{
        {"step", step},
        {"loss", loss},
        {"y_hat", y_hat},
        {"params", params.to_json()},
        {"grads", grads.to_json()}
    };
}

nlohmann::json TrainingResult::to_json() const {
    nlohmann::json j;
    j["initial_params"] = initial_params.to_json();
    j["final_params"] = final_params.to_json();
    j["learning_rate"] = learning_rate;
    j["total_steps"] = total_steps;
    j["grad_check"] = grad_check.to_json();
    j["csv_export"] = csv_export;

    nlohmann::json hist = nlohmann::json::array();
    for (const auto& s : history) {
        hist.push_back(s.to_json());
    }
    j["history"] = hist;
    return j;
}

// ── Forward Pass ────────────────────────────────────────────────────────────

ForwardState BackpropCore::forward(double x, double y, const ModelParameters& p) {
    ForwardState s;
    s.x = x;
    s.y = y;
    s.z = p.w * x + p.b;
    s.h = std::tanh(s.z);
    s.y_hat = p.v * s.h + p.c;
    double diff = s.y_hat - y;
    s.loss = 0.5 * diff * diff;
    return s;
}

// ── Backward Pass (Analytical Chain Rule) ───────────────────────────────────

Gradients BackpropCore::backward(const ForwardState& s, const ModelParameters& p) {
    Gradients g;

    // 1. Loss derivative w.r.t prediction: dJ/dy_hat = y_hat - y
    double delta = s.y_hat - s.y;

    // 2. Output bias: dJ/dc = dJ/dy_hat * dy_hat/dc = delta * 1
    g.dJ_dc = delta;

    // 3. Output weight: dJ/dv = dJ/dy_hat * dy_hat/dv = delta * h
    g.dJ_dv = delta * s.h;

    // 4. Hidden activation: dJ/dh = dJ/dy_hat * dy_hat/dh = delta * v
    double dJ_dh = delta * p.v;

    // 5. Pre-activation: dh/dz = 1 - tanh^2(z) = 1 - h^2
    //    dJ/dz = dJ/dh * dh/dz = delta * v * (1 - h^2)
    double dJ_dz = dJ_dh * (1.0 - s.h * s.h);

    // 6. Input weight: dJ/dw = dJ/dz * dz/dw = dJ/dz * x
    g.dJ_dw = dJ_dz * s.x;

    // 7. Hidden bias: dJ/db = dJ/dz * dz/db = dJ/dz * 1
    g.dJ_db = dJ_dz;

    return g;
}

// ── Numerical Gradients (Central Finite Differences) ────────────────────────

Gradients BackpropCore::numerical_gradients(
    double x, double y, const ModelParameters& p, double epsilon)
{
    Gradients num;
    const double two_eps = 2.0 * epsilon;

    // dJ/dw
    {
        ModelParameters p_plus = p;  p_plus.w += epsilon;
        ModelParameters p_minus = p; p_minus.w -= epsilon;
        num.dJ_dw = (forward(x, y, p_plus).loss - forward(x, y, p_minus).loss) / two_eps;
    }

    // dJ/db
    {
        ModelParameters p_plus = p;  p_plus.b += epsilon;
        ModelParameters p_minus = p; p_minus.b -= epsilon;
        num.dJ_db = (forward(x, y, p_plus).loss - forward(x, y, p_minus).loss) / two_eps;
    }

    // dJ/dv
    {
        ModelParameters p_plus = p;  p_plus.v += epsilon;
        ModelParameters p_minus = p; p_minus.v -= epsilon;
        num.dJ_dv = (forward(x, y, p_plus).loss - forward(x, y, p_minus).loss) / two_eps;
    }

    // dJ/dc
    {
        ModelParameters p_plus = p;  p_plus.c += epsilon;
        ModelParameters p_minus = p; p_minus.c -= epsilon;
        num.dJ_dc = (forward(x, y, p_plus).loss - forward(x, y, p_minus).loss) / two_eps;
    }

    return num;
}

// ── Gradient Verification ───────────────────────────────────────────────────

GradientCheckResult BackpropCore::verify_gradients(
    double x, double y, const ModelParameters& p, double epsilon, double tol)
{
    ForwardState fwd = forward(x, y, p);
    Gradients ana = backward(fwd, p);
    Gradients num = numerical_gradients(x, y, p, epsilon);

    GradientCheckResult res;
    res.epsilon = epsilon;
    res.tolerance = tol;
    res.all_passed = true;

    auto check_entry = [&](const std::string& name, double a, double n) {
        GradientCheckEntry entry;
        entry.param_name = name;
        entry.analytical = a;
        entry.numerical = n;
        entry.absolute_error = std::fabs(a - n);
        entry.passed = (entry.absolute_error <= tol);
        if (!entry.passed) res.all_passed = false;
        res.entries.push_back(entry);
    };

    check_entry("w", ana.dJ_dw, num.dJ_dw);
    check_entry("b", ana.dJ_db, num.dJ_db);
    check_entry("v", ana.dJ_dv, num.dJ_dv);
    check_entry("c", ana.dJ_dc, num.dJ_dc);

    return res;
}

// ── Training Loop (100 Steps) ───────────────────────────────────────────────

core::Result<TrainingResult> BackpropCore::train(
    double x,
    double y,
    ModelParameters initial_params,
    double lr,
    size_t steps,
    double epsilon)
{
    TrainingResult tr;
    tr.initial_params = initial_params;
    tr.learning_rate = lr;
    tr.total_steps = steps;
    tr.grad_check = verify_gradients(x, y, initial_params, epsilon);

    ModelParameters current = initial_params;

    std::ostringstream csv;
    csv << "step,loss,y_hat,w,b,v,c,dJ_dw,dJ_db,dJ_dv,dJ_dc\n";
    csv << std::setprecision(8) << std::fixed;

    for (size_t step = 0; step <= steps; ++step) {
        ForwardState state = forward(x, y, current);
        Gradients grads = backward(state, current);

        TrainingStep ts;
        ts.step = step;
        ts.loss = state.loss;
        ts.y_hat = state.y_hat;
        ts.params = current;
        ts.grads = grads;

        csv << step << ","
            << state.loss << ","
            << state.y_hat << ","
            << current.w << ","
            << current.b << ","
            << current.v << ","
            << current.c << ","
            << grads.dJ_dw << ","
            << grads.dJ_db << ","
            << grads.dJ_dv << ","
            << grads.dJ_dc << "\n";

        tr.history.push_back(std::move(ts));

        // Update weights using gradient descent for next step
        if (step < steps) {
            current.w -= lr * grads.dJ_dw;
            current.b -= lr * grads.dJ_db;
            current.v -= lr * grads.dJ_dv;
            current.c -= lr * grads.dJ_dc;
        }
    }

    tr.final_params = current;
    tr.csv_export = csv.str();
    return core::Result<TrainingResult>::ok(std::move(tr));
}

} // namespace enginex::neural

