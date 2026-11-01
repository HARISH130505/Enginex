#include "workbench/hardware/device_adapter.hpp"
#include "workbench/hardware/mock_adapters.hpp"
#include <cmath>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace enginex::hardware {

// ── Helpers ────────────────────────────────────────────────────────────────

std::string device_type_to_string(DeviceType t) {
    switch (t) {
        case DeviceType::MCU:  return "MCU";
        case DeviceType::FPGA: return "FPGA";
        case DeviceType::JTAG: return "JTAG";
        default:               return "UNKNOWN";
    }
}

std::string connection_state_to_string(ConnectionState s) {
    switch (s) {
        case ConnectionState::Disconnected: return "DISCONNECTED";
        case ConnectionState::Connecting:   return "CONNECTING";
        case ConnectionState::Connected:    return "CONNECTED";
        case ConnectionState::Fault:        return "FAULT";
        default:                            return "UNKNOWN";
    }
}

nlohmann::json TelemetrySample::to_json() const {
    // ISO 8601 timestamp
    auto tt = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream ts_oss;
    std::tm tm_buf{};
#ifdef _WIN32
    gmtime_s(&tm_buf, &tt);
#else
    gmtime_r(&tt, &tm_buf);
#endif
    ts_oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");

    return nlohmann::json{
        {"device_id", device_id},
        {"timestamp", ts_oss.str()},
        {"supply_voltage_V", supply_voltage_V},
        {"core_temperature_C", core_temperature_C},
        {"current_mA", current_mA},
        {"gpio_state", gpio_state},
        {"status_flags", status_flags},
        {"provenance", "SIMULATED — not real hardware telemetry"}
    };
}

// ── MockMCUAdapter (STM32F401RE) ────────────────────────────────────────────

MockMCUAdapter::MockMCUAdapter() : m_state(ConnectionState::Disconnected), m_tick(0) {}

ConnectionState MockMCUAdapter::state() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_state;
}

core::Result<void> MockMCUAdapter::connect() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_state == ConnectionState::Connected) {
        return core::Result<void>::fail(core::StatusCode::AlreadyExists, "STM32F401RE already connected");
    }
    m_state = ConnectionState::Connecting;
    // Simulated USART/SWD handshake: always succeeds on first attempt
    m_state = ConnectionState::Connected;
    m_tick = 0;
    return core::Result<void>::ok();
}

core::Result<void> MockMCUAdapter::disconnect() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_state = ConnectionState::Disconnected;
    return core::Result<void>::ok();
}

core::Result<void> MockMCUAdapter::recover() {
    std::lock_guard<std::mutex> lk(m_mutex);
    // Simulated SWD reset sequence: always recovers
    m_state = ConnectionState::Connected;
    m_tick = 0;
    return core::Result<void>::ok();
}

core::Result<TelemetrySample> MockMCUAdapter::read_telemetry() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_state != ConnectionState::Connected) {
        return core::Result<TelemetrySample>::fail(
            core::StatusCode::InvalidArgument, "STM32F401RE not connected");
    }

    m_tick++;

    // Inject fault every FAULT_PERIOD reads: simulate I2C timeout
    if ((m_tick % FAULT_PERIOD) == 0) {
        m_state = ConnectionState::Fault;
        return core::Result<TelemetrySample>::fail(
            core::StatusCode::Timeout,
            "[SIMULATED] STM32F401RE USART timeout at tick " + std::to_string(m_tick));
    }

    // Steady-state simulated readings with sinusoidal drift
    double phase = static_cast<double>(m_tick) * 0.3;
    TelemetrySample s;
    s.device_id         = id();
    s.timestamp         = std::chrono::system_clock::now();
    s.supply_voltage_V  = 3.30 + 0.02 * std::sin(phase);       // 3.28–3.32 V
    s.core_temperature_C = 45.0 + 3.5 * std::sin(phase * 0.5); // 41.5–48.5 °C
    s.current_mA        = 28.0 + 4.0 * std::cos(phase);        // 24–32 mA
    s.gpio_state        = (m_tick % 2 == 0) ? 0xA5U : 0x5AU;   // toggling GPIO
    s.status_flags      = "SIMULATED | UART_OK | ADC_RDY | TIM2_RUN";
    return core::Result<TelemetrySample>::ok(std::move(s));
}

nlohmann::json MockMCUAdapter::describe() const {
    return nlohmann::json{
        {"id", id()},
        {"label", label()},
        {"type", device_type_to_string(type())},
        {"state", connection_state_to_string(state())},
        {"chip", "STM32F401RE"},
        {"architecture", "ARM Cortex-M4 @ 84 MHz"},
        {"flash_KB", 512},
        {"ram_KB", 96},
        {"interface", "USART2 / SWD"},
        {"voltage_rail", "3.3 V"},
        {"fault_injection", "Simulated USART timeout every 7th telemetry read"},
        {"provenance", "SIMULATED"}
    };
}

// ── MockFPGAAdapter (Lattice iCE40UP5K) ────────────────────────────────────

MockFPGAAdapter::MockFPGAAdapter() : m_state(ConnectionState::Disconnected), m_tick(0) {}

ConnectionState MockFPGAAdapter::state() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_state;
}

core::Result<void> MockFPGAAdapter::connect() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_state == ConnectionState::Connected) {
        return core::Result<void>::fail(core::StatusCode::AlreadyExists, "iCE40UP5K already connected");
    }
    m_state = ConnectionState::Connecting;
    // Simulated SPI configuration bitstream load
    m_state = ConnectionState::Connected;
    m_tick = 0;
    return core::Result<void>::ok();
}

core::Result<void> MockFPGAAdapter::disconnect() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_state = ConnectionState::Disconnected;
    return core::Result<void>::ok();
}

core::Result<void> MockFPGAAdapter::recover() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_state = ConnectionState::Connected;
    m_tick = 0;
    return core::Result<void>::ok();
}

core::Result<TelemetrySample> MockFPGAAdapter::read_telemetry() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_state != ConnectionState::Connected) {
        return core::Result<TelemetrySample>::fail(
            core::StatusCode::InvalidArgument, "iCE40UP5K not connected");
    }

    m_tick++;
    double phase = static_cast<double>(m_tick) * 0.25;

    TelemetrySample s;
    s.device_id          = id();
    s.timestamp          = std::chrono::system_clock::now();
    s.supply_voltage_V   = 1.20 + 0.01 * std::sin(phase);       // 1.19–1.21 V core
    s.core_temperature_C = 38.0 + 2.0 * std::cos(phase * 0.4);  // 36–40 °C die temp
    s.current_mA         = 5.0 + 1.5 * std::sin(phase * 0.7);   // 3.5–6.5 mA
    s.gpio_state         = static_cast<uint32_t>(m_tick & 0xFF); // incrementing LUT output
    s.status_flags       = "SIMULATED | SPI_CONF_DONE | PLL_LOCK | IO_DRIVE_4mA";
    return core::Result<TelemetrySample>::ok(std::move(s));
}

nlohmann::json MockFPGAAdapter::describe() const {
    return nlohmann::json{
        {"id", id()},
        {"label", label()},
        {"type", device_type_to_string(type())},
        {"state", connection_state_to_string(state())},
        {"chip", "Lattice iCE40UP5K"},
        {"luts", 5280},
        {"plbs", 30},
        {"io_pins", 39},
        {"interface", "SPI (MOSI/MISO/SCK/SS_B)"},
        {"voltage_rail_core", "1.2 V"},
        {"voltage_rail_io", "3.3 V"},
        {"fault_injection", "None — benign device"},
        {"provenance", "SIMULATED"}
    };
}

// ── MockJTAGAdapter (FT2232H) ───────────────────────────────────────────────

MockJTAGAdapter::MockJTAGAdapter() : m_state(ConnectionState::Disconnected), m_tick(0) {}

ConnectionState MockJTAGAdapter::state() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_state;
}

core::Result<void> MockJTAGAdapter::connect() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_state == ConnectionState::Connected) {
        return core::Result<void>::fail(core::StatusCode::AlreadyExists, "FT2232H already connected");
    }
    m_state = ConnectionState::Connecting;
    // Simulated libftdi USB enumeration and MPSSE mode setup
    m_state = ConnectionState::Connected;
    m_tick = 0;
    return core::Result<void>::ok();
}

core::Result<void> MockJTAGAdapter::disconnect() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_state = ConnectionState::Disconnected;
    return core::Result<void>::ok();
}

core::Result<void> MockJTAGAdapter::recover() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_state = ConnectionState::Connected;
    m_tick = 0;
    return core::Result<void>::ok();
}

core::Result<TelemetrySample> MockJTAGAdapter::read_telemetry() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (m_state != ConnectionState::Connected) {
        return core::Result<TelemetrySample>::fail(
            core::StatusCode::InvalidArgument, "FT2232H not connected");
    }

    m_tick++;
    double phase = static_cast<double>(m_tick) * 0.2;

    // FT2232H telemetry: USB bus voltage, JTAG frequency, GPIO drive state
    TelemetrySample s;
    s.device_id          = id();
    s.timestamp          = std::chrono::system_clock::now();
    s.supply_voltage_V   = 5.00 + 0.05 * std::sin(phase);      // USB VBUS: 4.95–5.05 V
    s.core_temperature_C = 32.0 + 1.5 * std::sin(phase * 0.3); // 30.5–33.5 °C
    s.current_mA         = 50.0 + 8.0 * std::cos(phase * 0.6); // 42–58 mA USB current
    s.gpio_state         = 0x0FU;                               // TCK/TMS/TDI/TDO drive
    s.status_flags       = "SIMULATED | USB_HS | MPSSE_ACTIVE | JTAG_TCK_10MHz";
    return core::Result<TelemetrySample>::ok(std::move(s));
}

nlohmann::json MockJTAGAdapter::describe() const {
    return nlohmann::json{
        {"id", id()},
        {"label", label()},
        {"type", device_type_to_string(type())},
        {"state", connection_state_to_string(state())},
        {"chip", "FTDI FT2232H"},
        {"channels", 2},
        {"modes", "JTAG / SWD / UART / SPI / I2C via MPSSE"},
        {"interface", "USB 2.0 High-Speed (480 Mbps)"},
        {"jtag_clock_max_MHz", 30},
        {"voltage_rail", "5 V USB VBUS"},
        {"fault_injection", "None — probe device"},
        {"provenance", "SIMULATED"}
    };
}

} // namespace enginex::hardware

