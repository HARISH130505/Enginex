#pragma once

#include "workbench/hardware/device_adapter.hpp"
#include <atomic>
#include <mutex>
#include <cstdint>

namespace enginex::hardware {

// ── STM32F401RE Nucleo Mock (ARM Cortex-M4 @ 84 MHz) ──────────────────────
class MockMCUAdapter : public IDeviceAdapter {
public:
    MockMCUAdapter();

    std::string id() const override { return "MCU_STM32F401RE"; }
    std::string label() const override { return "STM32F401RE Nucleo (ARM Cortex-M4)"; }
    DeviceType type() const override { return DeviceType::MCU; }
    ConnectionState state() const override;

    core::Result<void> connect() override;
    core::Result<void> disconnect() override;
    core::Result<void> recover() override;
    core::Result<TelemetrySample> read_telemetry() override;
    nlohmann::json describe() const override;

private:
    mutable std::mutex m_mutex;
    ConnectionState m_state{ConnectionState::Disconnected};
    uint32_t m_tick{0};

    // Fault injection: every 7th telemetry read triggers a simulated timeout
    static constexpr uint32_t FAULT_PERIOD = 7;
};

// ── Lattice iCE40UP5K FPGA Mock ────────────────────────────────────────────
class MockFPGAAdapter : public IDeviceAdapter {
public:
    MockFPGAAdapter();

    std::string id() const override { return "FPGA_ICE40UP5K"; }
    std::string label() const override { return "Lattice iCE40UP5K FPGA"; }
    DeviceType type() const override { return DeviceType::FPGA; }
    ConnectionState state() const override;

    core::Result<void> connect() override;
    core::Result<void> disconnect() override;
    core::Result<void> recover() override;
    core::Result<TelemetrySample> read_telemetry() override;
    nlohmann::json describe() const override;

private:
    mutable std::mutex m_mutex;
    ConnectionState m_state{ConnectionState::Disconnected};
    uint32_t m_tick{0};
};

// ── FT2232H JTAG/SWD Debug Interface Mock ──────────────────────────────────
class MockJTAGAdapter : public IDeviceAdapter {
public:
    MockJTAGAdapter();

    std::string id() const override { return "JTAG_FT2232H"; }
    std::string label() const override { return "FTDI FT2232H JTAG/SWD Interface"; }
    DeviceType type() const override { return DeviceType::JTAG; }
    ConnectionState state() const override;

    core::Result<void> connect() override;
    core::Result<void> disconnect() override;
    core::Result<void> recover() override;
    core::Result<TelemetrySample> read_telemetry() override;
    nlohmann::json describe() const override;

private:
    mutable std::mutex m_mutex;
    ConnectionState m_state{ConnectionState::Disconnected};
    uint32_t m_tick{0};
};

} // namespace enginex::hardware

