# Embedded System Integration Design
## ENGINEX Workbench — Task E

### Scope and verification status

The ENGINEX hardware page and backend use mock adapters only. They do not access a physical board, USB device, driver, debug probe, or FPGA configuration interface. The design below is a **proposed architecture**, not a verified wiring diagram. No exact Nucleo revision, iCE40 board/revision, or FT2232H-based adapter board has been selected; connector contacts, adapter I/O voltage, power routing, and signal-level compatibility remain unverified.

### Proposed architecture

```text
Windows host
  ENGINEX browser UI <-> loopback C++ backend <-> mock device adapters
                                                     |
                       Proposed physical design only |
                                                     v
 Host USB <-> debug/bridge adapter <----> MCU (UART and either SWD or JTAG)
                           |
                           +-------------> FPGA configuration interface (SPI)

 A conventional JTAG TAP, if used, is a separate target interface. The
 iCE40 configuration path is not labelled as JTAG in this proposal.
```

### Interface and signal ownership

| Interface | Proposed endpoint/signals | Signal owner | Status |
|---|---|---|---|
| UART telemetry | MCU USART TX/RX to a USB-UART channel | MCU drives TX; host/adapter drives RX | Proposed. Confirm pin alternate functions, connector mapping, and voltage on the chosen boards. |
| ARM Serial Wire Debug | MCU SWDIO/SWCLK; optional reset | Debug host/adapter supplies SWCLK and controls bidirectional SWDIO; target responds on SWDIO | Proposed chip-level names are PA13/SWDIO and PA14/SWCLK for STM32F401RE. Board header contacts and adapter circuit are unverified. |
| IEEE 1149.1 JTAG | TCK, TMS, TDI, TDO; optional TRST | Adapter drives TCK/TMS/TDI; target drives TDO | Generic debug/test interface only. No specific JTAG target or connector is selected. |
| FPGA configuration | SPI/SysCONFIG signals such as SCK, SI/MOSI, SO/MISO, SS_B, and configuration status/reset signals | Exactly one selected configuration master drives the bus; FPGA returns data/status | Proposed for the iCE40 family. Confirm the selected device mode, pinout, and board routing before use. |

`SWCLK`/`SWDIO` are ARM SWD names; they are not aliases for JTAG `TCK`/`TMS`. A target configured for SWD must use the SWD protocol and its bidirectional data turnaround. If full JTAG is selected on an MCU, use its documented JTAG pin mapping instead; do not connect both interpretations simultaneously. The JTAG adapter is a host debug/test interface, not the application data path or a source of target power.

### Voltage, power, and grounding

| Domain | Proposed design constraint | Verification |
|---|---|---|
| Host USB VBUS | Nominal 5 V host supply to the adapter only unless an explicit target-power design allows otherwise. | No target power-sharing scheme selected. |
| MCU I/O | Treat 3.3 V as a design assumption only. Check the MCU datasheet, board schematic, and pin-specific tolerance. | Board/revision and connector mapping unverified. |
| iCE40 core | Use the device-required core rail (nominally 1.2 V for iCE40UP5K); do not apply this rail to I/O pins. | Verify regulator and sequencing on the exact board. |
| FPGA I/O bank | Depends on the board's VCCIO rail and configuration. Match the interface to that bank voltage. | Board/revision and VCCIO unverified. |
| Adapter I/O | FT2232H-class silicon does not establish the I/O voltage or protection of a finished adapter board. Check its schematic, VCCIO, target-reference sensing, and buffering. | Adapter model not selected. |
| Ground/reference | For a non-isolated connection, provide a compatible common signal reference before signal use. Preserve isolation if the selected adapter is designed to provide it. | Grounding and isolation topology unverified. |

Do not connect USB VBUS directly to target rails or drive signals into an unpowered target without verifying the board power design, pin absolute-maximum ratings, and back-powering behavior. Level shifting/buffering is selected from actual source and destination voltage domains, directionality, and bus timing; no fixed resistor or buffer value is prescribed here.

### Safe integration gate

Before any physical connection, identify the exact board revisions and obtain their manufacturer datasheets, schematics, and connector pinouts. Then document one configuration master per bus, voltage references, power sequencing, isolation/common-ground arrangement, level shifting, reset behavior, and connector orientation. With power off, verify the proposed connector mapping and continuity; use current-limited supplies and the adapter manufacturer's connection sequence. Physical hookup and electrical validation have not been performed for this project.

### Software representation

`MockMCUAdapter`, `MockFPGAAdapter`, and `MockJTAGAdapter` implement simulated device behavior. The user interface must continue to label their readings and operations as simulated. These mocks are not evidence of physical protocol support or board-level electrical validation.
