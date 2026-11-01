#include <iostream>
#include <stdexcept>
#include <string>
#include "workbench/hardware/mock_adapters.hpp"

#define TEST_ASSERT(cond, msg) \
    do { if (!(cond)) { throw std::runtime_error(std::string("FAIL: ") + (msg)); } } while(0)

using namespace enginex::hardware;

void test_mcu_connect_disconnect() {
    std::cout << "Running test_mcu_connect_disconnect...\n";
    MockMCUAdapter mcu;
    TEST_ASSERT(mcu.state() == ConnectionState::Disconnected, "MCU initial state must be DISCONNECTED");

    auto r = mcu.connect();
    TEST_ASSERT(r.is_ok(), "MCU connect must succeed");
    TEST_ASSERT(mcu.state() == ConnectionState::Connected, "MCU state must be CONNECTED after connect");

    auto dup = mcu.connect();
    TEST_ASSERT(dup.is_err(), "Duplicate connect must return error");

    auto r2 = mcu.disconnect();
    TEST_ASSERT(r2.is_ok(), "MCU disconnect must succeed");
    TEST_ASSERT(mcu.state() == ConnectionState::Disconnected, "MCU state must be DISCONNECTED after disconnect");

    std::cout << "  PASS: MCU connect/disconnect lifecycle verified.\n";
}

void test_mcu_fault_injection_and_recovery() {
    std::cout << "Running test_mcu_fault_injection_and_recovery...\n";
    MockMCUAdapter mcu;
    mcu.connect();

    bool fault_seen = false;
    for (int i = 0; i < 10; ++i) {
        auto t = mcu.read_telemetry();
        if (t.is_err()) {
            TEST_ASSERT(mcu.state() == ConnectionState::Fault, "State must be FAULT on telemetry failure");
            fault_seen = true;
            // Recover and verify
            auto rec = mcu.recover();
            TEST_ASSERT(rec.is_ok(), "Recovery must succeed");
            TEST_ASSERT(mcu.state() == ConnectionState::Connected, "State must be CONNECTED after recovery");
            break;
        }
        // Validate simulated readings are in plausible physical range
        const auto& s = t.value();
        TEST_ASSERT(s.supply_voltage_V > 3.0 && s.supply_voltage_V < 4.0, "MCU voltage out of range");
        TEST_ASSERT(s.core_temperature_C > 30.0 && s.core_temperature_C < 70.0, "MCU temp out of range");
    }
    TEST_ASSERT(fault_seen, "Fault injection must trigger within 10 telemetry reads");
    std::cout << "  PASS: MCU fault injection and simulated recovery verified.\n";
}

void test_fpga_telemetry_range() {
    std::cout << "Running test_fpga_telemetry_range...\n";
    MockFPGAAdapter fpga;
    fpga.connect();

    for (int i = 0; i < 5; ++i) {
        auto t = fpga.read_telemetry();
        TEST_ASSERT(t.is_ok(), "FPGA telemetry must not fail");
        const auto& s = t.value();
        TEST_ASSERT(s.supply_voltage_V > 1.0 && s.supply_voltage_V < 1.5, "FPGA core voltage out of range");
        TEST_ASSERT(s.core_temperature_C > 30.0 && s.core_temperature_C < 50.0, "FPGA temp out of range");
        TEST_ASSERT(s.device_id == "FPGA_ICE40UP5K", "FPGA device ID mismatch");
    }
    std::cout << "  PASS: FPGA telemetry range and device ID verified.\n";
}

void test_jtag_telemetry_and_describe() {
    std::cout << "Running test_jtag_telemetry_and_describe...\n";
    MockJTAGAdapter jtag;
    jtag.connect();

    auto t = jtag.read_telemetry();
    TEST_ASSERT(t.is_ok(), "JTAG telemetry must succeed");
    const auto& s = t.value();
    TEST_ASSERT(s.supply_voltage_V > 4.5 && s.supply_voltage_V < 5.5, "JTAG USB VBUS out of range");

    auto desc = jtag.describe();
    TEST_ASSERT(desc.contains("chip"), "JTAG describe must contain 'chip' key");
    TEST_ASSERT(desc["chip"] == "FTDI FT2232H", "JTAG chip mismatch");
    TEST_ASSERT(desc["provenance"] == "SIMULATED", "All devices must declare SIMULATED provenance");

    std::cout << "  PASS: JTAG telemetry, describe JSON, and SIMULATED provenance verified.\n";
}

void test_disconnected_telemetry_rejected() {
    std::cout << "Running test_disconnected_telemetry_rejected...\n";
    MockMCUAdapter mcu;
    // Do NOT connect
    auto t = mcu.read_telemetry();
    TEST_ASSERT(t.is_err(), "Telemetry on disconnected device must return error");
    std::cout << "  PASS: Disconnected device correctly rejects telemetry reads.\n";
}

void test_all_adapters_discovery_and_lifecycle() {
    std::cout << "Running test_all_adapters_discovery_and_lifecycle...\n";
    MockMCUAdapter mcu;
    MockFPGAAdapter fpga;
    MockJTAGAdapter jtag;

    IDeviceAdapter* adapters[] = {&mcu, &fpga, &jtag};

    for (auto* a : adapters) {
        // Discovery & capability reporting
        auto desc = a->describe();
        TEST_ASSERT(desc.contains("id"), "Device description must have id");
        TEST_ASSERT(desc.contains("label"), "Device description must have label");
        TEST_ASSERT(desc.contains("type"), "Device description must have type");
        TEST_ASSERT(desc.contains("state"), "Device description must have state");
        TEST_ASSERT(desc["provenance"] == "SIMULATED", "Provenance must be explicitly SIMULATED");
        TEST_ASSERT(a->state() == ConnectionState::Disconnected, "Initial state must be Disconnected");

        // Connect
        auto conn_res = a->connect();
        TEST_ASSERT(conn_res.is_ok(), "Connect must succeed");
        TEST_ASSERT(a->state() == ConnectionState::Connected, "State must be Connected");

        // At least one telemetry message
        auto t = a->read_telemetry();
        TEST_ASSERT(t.is_ok(), "Initial telemetry read must succeed");
        auto t_json = t.value().to_json();
        TEST_ASSERT(t_json.contains("device_id"), "Telemetry must have device_id");
        TEST_ASSERT(t_json["provenance"] == "SIMULATED — not real hardware telemetry", "Telemetry provenance must be SIMULATED");

        // Disconnect
        auto disc_res = a->disconnect();
        TEST_ASSERT(disc_res.is_ok(), "Disconnect must succeed");
        TEST_ASSERT(a->state() == ConnectionState::Disconnected, "State must be Disconnected after disconnect");

        // Reconnect
        auto reconn_res = a->connect();
        TEST_ASSERT(reconn_res.is_ok(), "Reconnect must succeed");
        TEST_ASSERT(a->state() == ConnectionState::Connected, "State must be Connected after reconnect");

        // Cleanup
        a->disconnect();
    }
    std::cout << "  PASS: Discovery, capability reporting, connect/disconnect, telemetry and reconnection verified across all adapters.\n";
}

int main() {
    std::cout << "=== ENGINEX Mock Hardware Adapter Tests ===\n";
    try {
        test_mcu_connect_disconnect();
        test_mcu_fault_injection_and_recovery();
        test_fpga_telemetry_range();
        test_jtag_telemetry_and_describe();
        test_disconnected_telemetry_rejected();
        test_all_adapters_discovery_and_lifecycle();
        std::cout << "\nALL 6 HARDWARE ADAPTER TESTS PASSED!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << "\n";
        return 1;
    }
}

