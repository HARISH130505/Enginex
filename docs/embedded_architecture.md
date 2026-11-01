# Embedded-System Interface Proposal and Verification Limits

> **Implementation status:** ENGINEX currently demonstrates mock hardware only. No physical MCU, FPGA, FTDI adapter, wiring, or protocol transaction has been tested. Everything below is a design proposal pending verification against the exact selected device and board revisions.

## 1. Architecture

```text
                         SOFTWARE (implemented with mocks)
  Windows host -> ENGINEX Workbench -> MockMCUAdapter
                                     -> MockFPGAAdapter
                                     -> MockJTAGAdapter

                    PHYSICAL TOPOLOGY (proposed only)
  Host USB -> FT2232H-class adapter/bridge
                 |-- UART channel <-> MCU UART
                 |-- SWD or JTAG  <-> MCU debug port (choose one)
                 |-- SPI/SysCONFIG <-> FPGA configuration port

  A conventional JTAG TAP, if present, is a separate interface. Do not
  label FPGA SPI signals as JTAG or assume the selected FPGA board has a TAP.
```

The FT2232H is a USB interface IC with configurable I/O/MPSSE functions, not by itself a complete, voltage-safe debugger. A finished adapter's connectors, buffers, target-voltage sensing, power arrangement, and protection depend on its board design. No particular adapter-board product is selected here.

## 2. Protocol names and ownership

| Protocol | Signals | Who drives the signals | Proposed use |
|---|---|---|---|
| ARM SWD | `SWCLK`, bidirectional `SWDIO`, optional `nRESET` | Debug host/adapter drives `SWCLK`; `SWDIO` changes direction for request/response phases; reset is driven only through a compatible reset circuit | Proposed MCU debug connection. For STM32F401RE, PA13/PA14 are candidate chip-level SWD signals; verify the selected MCU package and board header. |
| IEEE 1149.1 JTAG | `TCK`, `TMS`, `TDI`, `TDO`, optional `TRST` | Adapter drives `TCK`, `TMS`, and `TDI`; target drives `TDO` | Generic TAP debug/test connection. No target TAP or connector has been selected. |
| UART | `TX`, `RX`, optional flow control | Each endpoint drives its own TX; RX is an input | Proposed MCU telemetry/command channel over a USB-UART interface. Confirm MCU alternate functions and board routing. |
| FPGA SPI/SysCONFIG | Mode-dependent clock, data, select, reset/status signals (for example `SCK`, `SI`, `SO`, `SS_B`) | One selected configuration master drives the bus; the FPGA returns data/status as defined by the selected mode | Proposed FPGA configuration interface. Confirm supported mode, package pinout, and board routing from the exact device and board documentation. |

`SWCLK` is not `TCK`, and `SWDIO` is not `TMS`. The names describe different protocols and direction behavior. Do not wire SWD by substituting JTAG labels or assume a two-wire JTAG interface. The MCU debug interface should be configured as either SWD or full JTAG using the selected MCU documentation and debug software; shared MCU pins must not be assigned to conflicting functions at the same time.

For FPGA configuration, this proposal uses SPI/SysCONFIG terminology, not JTAG terminology. Before claiming that a JTAG path exists for an FPGA, verify a TAP and its pin mapping in the selected device and board documentation. The target board has not been selected.

### JTAG adapter purpose

A JTAG adapter is a debug/test bridge between host software and a target's TAP: it produces the TAP clock/control/data-input signals and samples target data output. It does not replace application communication such as UART and does not inherently supply safe target power. An FT2232H-based adapter can expose MPSSE-capable pins, but the actual adapter circuit and software protocol support must be verified for the chosen board. The project implements a `MockJTAGAdapter`, not that physical transport.

## 3. Proposed signal map (not a wiring instruction)

The following is a protocol-level map. It intentionally does not assign FTDI package pins, board connector numbers, or cable contacts because no adapter or FPGA board revision is selected.

| Link | Signal names | Direction at target | Voltage/connector status |
|---|---|---|---|
| Host adapter to MCU SWD | `SWCLK`, `SWDIO`, optional `nRESET` | Clock/reset into MCU; `SWDIO` bidirectional | Candidate MCU signal names only; voltage and physical connector unverified. |
| MCU to host adapter UART | MCU `TX` to adapter `RX`; adapter `TX` to MCU `RX` | One TX driver per direction | Candidate telemetry path; UART instance, alternate functions, connector, and logic levels unverified. |
| Host adapter to FPGA configuration | Mode-specific `SCK`, `SI/MOSI`, `SO/MISO`, `SS_B`, reset/status | Master drives clock/data/select; target returns data/status | Proposed SPI/SysCONFIG; exact mode and board pins unverified. |
| Host adapter to generic JTAG TAP | `TCK`, `TMS`, `TDI`, `TDO`, optional `TRST` | `TCK/TMS/TDI` into TAP; `TDO` out | No physical TAP target or connector selected. |

There are no verified connector pin numbers in this document. Do not use prior references to “CN7 pin 13/15”, “J2 pins”, FTDI `ADBUS`/`BDBUS` numbers, or package-pad numbers as physical instructions without confirming them against the exact current manufacturer schematic and board revision.

## 4. Voltage, power, and grounding

| Domain | Design consideration | Current evidence |
|---|---|---|
| Host USB VBUS | Nominal 5 V USB supply is not automatically a target supply. Do not connect it to MCU/FPGA rails without a documented power budget and board-approved path. | Physical power topology not selected. |
| MCU supply/I/O | 3.3 V is a proposed logic domain only. Check the exact STM32 datasheet, board regulator, pin limits, and any onboard debugger routing. | Board/revision and pin-level compatibility unverified. |
| iCE40UP5K core | The device core supply is nominally 1.2 V; it is separate from I/O-bank supply. | Confirm against the selected device variant and board schematic before wiring. |
| FPGA I/O banks | Voltage is determined by the bank's VCCIO rail and board design; do not infer it from the core voltage or a generic board name. | Exact VCCIO and exposed pins unverified. |
| Adapter I/O | An FT2232H-based board's VCCIO, target reference, level shifting, and protection depend on the adapter schematic. | No adapter-board model selected. |
| Ground/reference | A non-isolated signal link requires a compatible reference and an intentional return path. Follow any selected adapter isolation design. | Grounding/isolation topology not physically checked. |

Level shifting is not assumed to be present in the FT2232H IC or in an unspecified adapter. Select buffers only after checking both sides' voltage limits, signal directions, and timing. Avoid driving an unpowered target and avoid unintended rail backfeed. The previous draft's fixed 3.3 V levels, “VERIFIED” pin labels, and universal resistor/buffer recommendations are withdrawn.

## 5. Safe integration review gate

Physical integration remains blocked until the candidate selects exact boards and adapter hardware and records:

1. Manufacturer part numbers, board revisions, datasheets, schematics, and connector pinouts.
2. Whether the MCU is operated over SWD or JTAG, and the corresponding pin mux/debug configuration.
3. The FPGA configuration mode and exact package/board signal routing.
4. Adapter VCCIO, target-reference sensing, level shifting, isolation, and reset circuitry.
5. Target power source, current limits, power-up/down sequencing, common reference or isolation, and back-powering safeguards.
6. Connector orientation, continuity checks with power off, and an electrically reviewed, revision-specific wiring diagram.

These are verification requirements, not claims that a physical connection has been made. No board hookup or electrical test has been performed for ENGINEX.

## 6. Manufacturer documentation to consult

Use the revision applicable to the selected hardware; the references below are pointers, not evidence that a specific board pinout was checked:

- STMicroelectronics STM32F401x datasheet and Nucleo-64 board user manual/schematic (for example, DS10086 and UM1724 where applicable).
- Lattice iCE40 UltraPlus family datasheet and configuration documentation (for example, DS1048 and TN1248), plus the exact evaluation-board revision manual.
- FTDI FT2232H datasheet and the selected adapter board's own schematic; the IC datasheet alone does not establish adapter-board wiring or voltage compatibility.
- ARM Debug Interface Architecture documentation for SWD protocol behavior.

## 7. Software implementation boundary

The mock adapter interface exposes simulated discovery/description, connect/disconnect, telemetry, and recovery operations. Mock telemetry is not measured hardware data. Preserve visible `SIMULATED` labels in the UI and API, and do not describe the mock tests as electrical, USB-driver, JTAG, SWD, or FPGA-programming validation.