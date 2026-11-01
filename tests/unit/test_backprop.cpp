#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>
#include "workbench/neural/backprop.hpp"

#define TEST_ASSERT(cond, msg) \
    do { if (!(cond)) { throw std::runtime_error(std::string("FAIL: ") + (msg)); } } while(0)

using namespace enginex::neural;

void test_forward_initial_values() {
    std::cout << "Running test_forward_initial_values...\n";
    ModelParameters p; // w=0.5, b=0, v=1, c=0
    ForwardState s = BackpropCore::forward(1.0, 2.0, p);

    // z = 0.5 * 1.0 + 0.0 = 0.5
    TEST_ASSERT(std::fabs(s.z - 0.5) < 1e-12, "z != 0.5");

    // h = tanh(0.5) ~ 0.4621171572600097585
    double expected_h = std::tanh(0.5);
    TEST_ASSERT(std::fabs(s.h - expected_h) < 1e-12, "h != tanh(0.5)");

    // y_hat = 1.0 * h + 0.0 = h
    TEST_ASSERT(std::fabs(s.y_hat - expected_h) < 1e-12, "y_hat != h");

    // diff = y_hat - 2.0 = -1.5378828427399902
    // loss = 0.5 * diff^2 ~ 1.18254181977
    double diff = expected_h - 2.0;
    double expected_loss = 0.5 * diff * diff;
    TEST_ASSERT(std::fabs(s.loss - expected_loss) < 1e-12, "loss mismatch");

    std::cout << "  PASS: Initial forward pass values verified (loss = " << s.loss << ").\n";
}

void test_analytical_gradient_formulas() {
    std::cout << "Running test_analytical_gradient_formulas...\n";
    ModelParameters p;
    ForwardState s = BackpropCore::forward(1.0, 2.0, p);
    Gradients g = BackpropCore::backward(s, p);

    double delta = s.y_hat - 2.0;
    double h = s.h;

    // dJ/dc = delta
    TEST_ASSERT(std::fabs(g.dJ_dc - delta) < 1e-12, "dJ_dc mismatch");

    // dJ/dv = delta * h
    TEST_ASSERT(std::fabs(g.dJ_dv - (delta * h)) < 1e-12, "dJ_dv mismatch");

    // dJ/dz = delta * v * (1 - h^2)
    double expected_dJ_dz = delta * p.v * (1.0 - h * h);

    // dJ/dw = dJ/dz * x = dJ/dz * 1.0
    TEST_ASSERT(std::fabs(g.dJ_dw - expected_dJ_dz) < 1e-12, "dJ_dw mismatch");

    // dJ/db = dJ/dz * 1.0
    TEST_ASSERT(std::fabs(g.dJ_db - expected_dJ_dz) < 1e-12, "dJ_db mismatch");

    std::cout << "  PASS: Analytical gradient formulas match mathematical chain rule.\n";
}

void test_central_finite_difference_verification() {
    std::cout << "Running test_central_finite_difference_verification...\n";
    ModelParameters p;
    double eps = 1e-6;
    double tol = 1e-5;

    GradientCheckResult res = BackpropCore::verify_gradients(1.0, 2.0, p, eps, tol);
    TEST_ASSERT(res.all_passed, "Gradient check must pass for all parameters");
    TEST_ASSERT(res.entries.size() == 4, "Expected 4 parameter gradient check entries");

    for (const auto& e : res.entries) {
        TEST_ASSERT(e.passed, "Parameter " + e.param_name + " failed gradient check");
        TEST_ASSERT(e.absolute_error <= tol, "Error for " + e.param_name + " exceeds tolerance");
        std::cout << "    [" << e.param_name << "] Analytical: " << e.analytical
                  << " | Numerical: " << e.numerical
                  << " | Error: " << e.absolute_error << " (PASS)\n";
    }

    std::cout << "  PASS: Central finite differences gradient check verified (all errors <= 1e-5).\n";
}

void test_100_step_training_monotonic_loss_decrease() {
    std::cout << "Running test_100_step_training_monotonic_loss_decrease...\n";
    auto res = BackpropCore::train(1.0, 2.0, ModelParameters{}, 0.1, 100);
    TEST_ASSERT(res.is_ok(), "train() failed");

    const auto& tr = res.value();
    TEST_ASSERT(tr.history.size() == 101, "History must contain 101 steps (0..100)");

    double initial_loss = tr.history[0].loss;
    double final_loss = tr.history[100].loss;

    TEST_ASSERT(final_loss < initial_loss, "Final loss must be less than initial loss");
    TEST_ASSERT(final_loss < 0.05, "100 steps with lr=0.1 should reduce loss below 0.05");

    // Check monotonic decrease
    for (size_t i = 1; i < tr.history.size(); ++i) {
        TEST_ASSERT(tr.history[i].loss <= tr.history[i - 1].loss + 1e-10,
            "Loss should decrease monotonically for single-sample regression");
    }

    std::cout << "  PASS: Loss decreased from " << initial_loss << " to " 
              << final_loss << " (prediction y_hat reached " << tr.history[100].y_hat << ").\n";
}

void test_csv_export_format() {
    std::cout << "Running test_csv_export_format...\n";
    auto res = BackpropCore::train(1.0, 2.0, ModelParameters{}, 0.1, 10);
    TEST_ASSERT(res.is_ok(), "train() failed");

    const std::string& csv = res.value().csv_export;
    std::string expected_header = "step,loss,y_hat,w,b,v,c,dJ_dw,dJ_db,dJ_dv,dJ_dc\n";
    TEST_ASSERT(csv.rfind(expected_header, 0) == 0, "CSV header mismatch");

    std::cout << "  PASS: CSV export format verified.\n";
}

void test_loss_progression_reference_milestones() {
    std::cout << "Running test_loss_progression_reference_milestones...\n";
    auto res = BackpropCore::train(1.0, 2.0, ModelParameters{}, 0.1, 100);
    TEST_ASSERT(res.is_ok(), "train() failed");

    const auto& tr = res.value();
    TEST_ASSERT(tr.history.size() == 101, "History must contain 101 steps");

    // Reference targets:
    // Step 0: ~ 1.182541819
    // Step 10: ~ 0.010211016
    // Step 50: ~ 1.2761e-10
    // Step 100: ~ 1.7026e-20
    double loss_0 = tr.history[0].loss;
    double loss_10 = tr.history[10].loss;
    double loss_50 = tr.history[50].loss;
    double loss_100 = tr.history[100].loss;

    std::cout << "    Step 0 loss:   " << loss_0 << "\n";
    std::cout << "    Step 10 loss:  " << loss_10 << "\n";
    std::cout << "    Step 50 loss:  " << loss_50 << "\n";
    std::cout << "    Step 100 loss: " << loss_100 << "\n";

    TEST_ASSERT(std::fabs(loss_0 - 1.182541819) < 1e-6, "Step 0 loss mismatch");
    TEST_ASSERT(std::fabs(loss_10 - 0.010211016) < 1e-4, "Step 10 loss mismatch");
    TEST_ASSERT(loss_50 < 1.0e-8, "Step 50 loss should be around 1e-10");
    TEST_ASSERT(loss_100 < 1.0e-15, "Step 100 loss should be around 1e-20");

    // Verify initial simultaneous update matches analytical targets:
    // w = 0.6209464475, b = 0.1209464475, v = 1.0710682047, c = 0.1537882843
    const auto& p1 = tr.history[1].params;
    TEST_ASSERT(std::fabs(p1.w - 0.6209464475) < 1e-7, "Step 1 w mismatch");
    TEST_ASSERT(std::fabs(p1.b - 0.1209464475) < 1e-7, "Step 1 b mismatch");
    TEST_ASSERT(std::fabs(p1.v - 1.0710682047) < 1e-7, "Step 1 v mismatch");
    TEST_ASSERT(std::fabs(p1.c - 0.1537882843) < 1e-7, "Step 1 c mismatch");

    std::cout << "  PASS: Loss progression milestones and step 1 simultaneous updates verified.\n";
}

int main() {
    std::cout << "=== ENGINEX Neural Backpropagation Unit Tests ===\n";
    try {
        test_forward_initial_values();
        test_analytical_gradient_formulas();
        test_central_finite_difference_verification();
        test_100_step_training_monotonic_loss_decrease();
        test_csv_export_format();
        test_loss_progression_reference_milestones();
        std::cout << "\nALL 6 BACKPROPAGATION TESTS PASSED!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << "\n";
        return 1;
    }
}

