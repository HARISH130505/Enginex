#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include "workbench/core/result.hpp"
#include "workbench/core/json.hpp"

namespace enginex::hardware {

// Device type enum
enum class DeviceType {
    MCU,   // Microcontroller (ARM Cortex-M4 — STM32F401RE)
    FPGA,  // FPGA (Lattice iCE40UP5K)
    JTAG   // JTAG Debug Interface (FT2232H)
};

std::string device_type_to_string(DeviceType t);

// Connection state machine
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Fault
};

std::string connection_state_to_string(ConnectionState s);

// Simulated telemetry data sample
struct TelemetrySample {
    std::string device_id;
    std::chrono::system_clock::time_point timestamp;
    double supply_voltage_V = 0.0;    // V
    double core_temperature_C = 0.0;  // °C (die temp)
    double current_mA = 0.0;          // mA
    uint32_t gpio_state = 0x00;       // GPIO port snapshot
    std::string status_flags;         // Human-readable status

    nlohmann::json to_json() const;
};

// Abstract device adapter interface
class IDeviceAdapter {
public:
    virtual ~IDeviceAdapter() = default;

    virtual std::string id() const = 0;
    virtual std::string label() const = 0;
    virtual DeviceType type() const = 0;
    virtual ConnectionState state() const = 0;

    // Attempt connection with simulated timeout/retry
    virtual core::Result<void> connect() = 0;
    virtual core::Result<void> disconnect() = 0;

    // Attempt reconnect after simulated fault with recovery
    virtual core::Result<void> recover() = 0;

    // Read telemetry snapshot
    virtual core::Result<TelemetrySample> read_telemetry() = 0;

    virtual nlohmann::json describe() const = 0;
};

} // namespace enginex::hardware

