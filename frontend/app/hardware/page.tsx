"use client";

import React from "react";
import { Radio, Cpu, Layers, Terminal, AlertTriangle, RefreshCw } from "lucide-react";

const devices = [
  {
    id: "mcu",
    name: "Microcontroller Board",
    model: "STM32F401RE (ARM Cortex-M4)",
    protocol: "USB CDC / Virtual COM Port (115200 baud)",
    voltage: "3.3V Logic (5V USB Powered)",
    status: "DISCONNECTED",
    badge: "SIMULATED",
    desc: "Performs embedded real-time sensor sampling and PID thermal actuation loop."
  },
  {
    id: "fpga",
    name: "FPGA Board",
    model: "Lattice iCE40UP5K",
    protocol: "High-Speed SPI / UART Bridged",
    voltage: "1.2V Core / 3.3V I/O",
    status: "DISCONNECTED",
    badge: "SIMULATED",
    desc: "Accelerates parallel matrix arithmetic and high-bandwidth telemetry processing."
  },
  {
    id: "jtag",
    name: "JTAG Debug Adapter",
    model: "FT2232H / ST-Link V3",
    protocol: "Standard JTAG (TCK, TMS, TDI, TDO, nSRST)",
    voltage: "1.8V – 3.3V Target Reference",
    status: "DISCONNECTED",
    badge: "SIMULATED",
    desc: "Dedicated hardware debugging and in-system programming interface (not a general compute node)."
  }
];

export default function HardwarePage() {
  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-4 border-b border-gray-800">
        <div>
          <div className="flex items-center gap-2">
            <h1 className="text-2xl font-bold text-white tracking-tight">Embedded System Integration Design</h1>
            <span className="text-xs font-mono px-2 py-0.5 rounded bg-purple-900/60 text-purple-300 border border-purple-700/50">Task E</span>
          </div>
          <p className="text-xs text-gray-400 mt-1">
            Host-connected workbench architecture with three C++ mock adapters, simulated fault injection, and retry recovery.
          </p>
        </div>
      </div>

      {/* 3 Device Cards */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-5">
        {devices.map((dev) => (
          <div key={dev.id} className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 flex flex-col justify-between space-y-4">
            <div>
              <div className="flex items-center justify-between mb-2">
                <span className="text-[10px] font-mono font-bold px-2 py-0.5 rounded bg-purple-950/80 text-purple-300 border border-purple-800/60">
                  {dev.badge}
                </span>
                <span className="text-[10px] font-mono px-2 py-0.5 rounded bg-gray-800 text-gray-400">
                  {dev.status}
                </span>
              </div>
              <h3 className="text-sm font-bold text-white">{dev.name}</h3>
              <div className="text-xs font-mono text-purple-400 mt-0.5">{dev.model}</div>
              <p className="text-xs text-gray-400 mt-2 leading-relaxed">{dev.desc}</p>
            </div>

            <div className="space-y-2 pt-3 border-t border-gray-800 text-[11px] font-mono text-gray-400">
              <div><span className="text-gray-500">Link:</span> {dev.protocol}</div>
              <div><span className="text-gray-500">Domain:</span> {dev.voltage}</div>
              <button className="w-full mt-2 py-1.5 px-3 rounded bg-gray-800 hover:bg-gray-700 text-xs font-medium text-gray-200 transition-colors">
                Connect Device
              </button>
            </div>
          </div>
        ))}
      </div>

      {/* Integration Design & Wiring Table Placeholder */}
      <div className="rounded-xl bg-[#0c1220] p-6 border border-gray-800 space-y-3">
        <h3 className="text-sm font-bold text-white flex items-center gap-2">
          <Layers className="w-4 h-4 text-purple-400" />
          <span>Interface Architecture & Pin-Voltage Isolation Matrix</span>
        </h3>
        <p className="text-xs text-gray-400">
          Phase G5 will document the full wiring topology, ground references, level-shifting constraints, and JTAG signal ownership.
        </p>
      </div>
    </div>
  );
}

