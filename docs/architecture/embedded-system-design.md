# Embedded System Integration Design
## ENGINEX Workbench — Task E: Hardware Interface Architecture

> **Provenance Notice:** All hardware descriptions, schematics, pin tables, and telemetry values in this document describe a **simulated** embedded system design. No physical hardware is connected. All C++ mock adapters produce synthetic, deterministically seeded readings for demonstration and evaluation purposes only.

---

## 1. System Block Diagram

```
┌──────────────────────────────────────────────────────────────────────┐
│                    Host PC (Windows 11 x64)                          │
│                                                                      │
│   ┌───────────────────────────────────────────────────────────┐      │
│   │  ENGINEX Workbench (backend_server.exe)                  │      │
│   │   └── IDeviceAdapter registry                            │      │
│   │         ├── MockMCUAdapter   (STM32F401RE)               │      │
│   │         ├── MockFPGAAdapter  (Lattice iCE40UP5K)         │      │
│   │         └── MockJTAGAdapter  (FT2232H)                   │      │
│   └───────────────────────────────────────────────────────────┘      │
│             │ USB / UART / SPI / JTAG                                │
└─────────────┼────────────────────────────────────────────────────────┘
              │ [Simulated on-host — no physical cables required]
   ┌──────────▼──────────────────────────────────────────────┐
   │                 Embedded System Under Test               │
   │                                                          │
   │  ┌─────────────────┐    ┌──────────────────┐           │
   │  │  STM32F401RE    │    │  Lattice iCE40    │           │
   │  │  Nucleo Board   │    │  UP5K FPGA Board  │           │
   │  │  ARM Cortex-M4  │────│  5280 LUTs        │           │
   │  │  84 MHz, 3.3V   │    │  SPI config bus   │           │
   │  │  UART2 @ 115200 │    │  1.2V core rail   │           │
   │  └────────┬────────┘    └────────┬──────────┘           │
   │           │                      │                       │
   │  ┌────────▼──────────────────────▼──────────┐           │
   │  │          FTDI FT2232H JTAG Probe          │           │
   │  │   USB 2.0 HS ↔ JTAG/SWD/MPSSE Interface  │           │
   │  │   TCK/TMS/TDI/TDO/TRST — 10 MHz max      │           │
   │  └──────────────────────────────────────────┘           │
   └──────────────────────────────────────────────────────────┘
```

---

## 2. Device Specifications

### Device 1: STM32F401RE Nucleo Board (MCU)

| Property | Value |
|---|---|
| Chip | STM32F401RE |
| Architecture | ARM Cortex-M4 @ 84 MHz |
| Flash | 512 KB |
| RAM | 96 KB SRAM |
| Primary Interface | USART2 @ 115,200 baud, 8N1 |
| Debug Interface | ARM SWD (2-wire) |
| Supply Voltage | 3.3 V regulated (from ST-LINK USB) |
| On-board LED | PA5 (LD2, GPIO push-pull) |
| Fault Simulation | USART timeout every 7th telemetry read |
| Recovery Method | SWD soft-reset (AIRCR.SYSRESETREQ) |

### Device 2: Lattice iCE40UP5K FPGA

| Property | Value |
|---|---|
| Chip | Lattice iCE40UP5K |
| Logic Units | 5,280 LUTs / 8 DSPs |
| PLBs (Physical Logic Blocks) | 30 |
| User I/O Pins | 39 |
| Configuration Interface | SPI (Passive Serial, MOSI/MISO/SCK/SS_B) |
| Core Supply | 1.2 V (VCCCORE) |
| I/O Supply | 3.3 V (VCCIO) |
| Internal Oscillator | 48 MHz (HFOSC) |
| BRAM | 1 Mbit |
| Fault Simulation | None — passive configuration device |

### Device 3: FTDI FT2232H JTAG/SWD Debug Probe

| Property | Value |
|---|---|
| Chip | FTDI FT2232H |
| Channels | 2 (MPSSE-capable Channel A, UART Channel B) |
| USB Speed | High-Speed 2.0 (480 Mbps) |
| Modes | JTAG, SWD, UART, SPI, I2C (via MPSSE engine) |
| JTAG Clock | Up to 30 MHz |
| Supply | 5 V VBUS (USB powered) |
| Voltage Tolerance | 1.8 V – 5 V target voltage sense (VREF) |
| Role | Bridges host USB → MCU SWD + FPGA JTAG boundary scan |

---

## 3. Pin and Voltage Table

| Net Name | Source | Destination | Voltage Level | Direction | Description |
|---|---|---|---|---|---|
| USART2_TX | STM32 PA2 | FT2232H CH-B RXD | 3.3 V | MCU → Host | Debug/telemetry UART transmit |
| USART2_RX | FT2232H CH-B TXD | STM32 PA3 | 3.3 V | Host → MCU | Debug UART receive |
| SWD_CLK | FT2232H CH-A TCK | STM32 PA14 | 3.3 V | Host → MCU | SWD clock |
| SWD_IO | FT2232H CH-A TMS | STM32 PA13 | 3.3 V | Bidirectional | SWD data |
| SPI_SCK | STM32 PB3 | iCE40 SCK | 3.3 V | MCU → FPGA | SPI configuration clock |
| SPI_MOSI | STM32 PB5 | iCE40 MOSI (SI) | 3.3 V | MCU → FPGA | SPI data out |
| SPI_MISO | iCE40 MISO (SO) | STM32 PB4 | 3.3 V | FPGA → MCU | SPI readback |
| FPGA_SS_B | STM32 PB6 | iCE40 SS_B | 3.3 V | MCU → FPGA | SPI chip select |
| JTAG_TCK | FT2232H CH-A ADBUS0 | iCE40 TCK | 3.3 V | Host → FPGA | JTAG clock |
| JTAG_TMS | FT2232H CH-A ADBUS3 | iCE40 TMS | 3.3 V | Host → FPGA | JTAG TAP controller |
| JTAG_TDI | FT2232H CH-A ADBUS1 | iCE40 TDI | 3.3 V | Host → FPGA | JTAG data in |
| JTAG_TDO | iCE40 TDO | FT2232H CH-A ADBUS2 | 3.3 V | FPGA → Host | JTAG data out |
| VDD_3V3 | LDO regulator | STM32 VDD, iCE40 VCCIO | 3.3 V | Supply | I/O power rail |
| VDD_1V2 | DC-DC converter | iCE40 VCCCORE | 1.2 V | Supply | FPGA core power |
| GND | Common | All | 0 V | Reference | Ground |

---

## 4. Software Interface Design (C++17)

### 4.1 IDeviceAdapter Interface

The `IDeviceAdapter` abstract class defines the contract for all hardware adapters:

```cpp
class IDeviceAdapter {
public:
    virtual core::Result<void> connect() = 0;
    virtual core::Result<void> disconnect() = 0;
    virtual core::Result<void> recover() = 0;
    virtual core::Result<TelemetrySample> read_telemetry() = 0;
    virtual nlohmann::json describe() const = 0;
};
```

### 4.2 Fault Injection & Recovery

The `MockMCUAdapter` injects a simulated USART timeout every 7th telemetry read, transitioning the adapter to `ConnectionState::Fault`. The HTTP server auto-recovers via `IDeviceAdapter::recover()` before returning the error response and on the next telemetry call.

### 4.3 REST API Endpoints

| Method | Route | Description |
|---|---|---|
| `GET` | `/api/v1/hardware/devices` | List all devices with describe JSON |
| `POST` | `/api/v1/hardware/{id}/connect` | Connect a device |
| `POST` | `/api/v1/hardware/{id}/disconnect` | Disconnect a device |
| `GET` | `/api/v1/hardware/{id}/telemetry` | Read telemetry (auto-recover on fault) |

---

## 5. All Data is SIMULATED

> **IMPORTANT:** All telemetry values, GPIO states, voltages, and temperatures produced by the mock adapters are computed from trigonometric functions of a monotonically increasing tick counter. No real hardware, driver, or OS interface (libftdi, libusb, STM32Cube HAL, iCEburn, etc.) is invoked. The label `SIMULATED` appears in every telemetry JSON response and in every adapter's `describe()` output.

