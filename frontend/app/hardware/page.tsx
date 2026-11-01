"use client";

import React, { useState, useEffect, useRef, useCallback } from "react";
import { 
  Radio, 
  Cpu, 
  Layers, 
  Terminal, 
  AlertTriangle, 
  RefreshCw, 
  CheckCircle2, 
  Power, 
  Activity,
  Zap,
  Thermometer,
  ShieldAlert,
  Info
} from "lucide-react";
import { getApiBaseUrl } from "@/lib/api/client";

interface DeviceInfo {
  id: string;
  label: string;
  type: string;
  state: string;
  chip?: string;
  architecture?: string;
  interface?: string;
  voltage_rail?: string;
  voltage_rail_core?: string;
  voltage_rail_io?: string;
  fault_injection?: string;
  provenance: string;
}

interface TelemetryData {
  device_id: string;
  timestamp: string;
  supply_voltage_V: number;
  core_temperature_C: number;
  current_mA: number;
  gpio_state: number;
  status_flags: string;
  provenance: string;
}

export default function HardwarePage() {
  const [devices, setDevices] = useState<DeviceInfo[]>([]);
  const [telemetry, setTelemetry] = useState<Record<string, TelemetryData>>({});
  const [loadingDevice, setLoadingDevice] = useState<string | null>(null);
  const [faultAlert, setFaultAlert] = useState<string | null>(null);
  const [autoPoll, setAutoPoll] = useState<boolean>(true);

  // Fetch device list
  const fetchDevices = useCallback(async () => {
    try {
      const base = getApiBaseUrl();
      const res = await fetch(`${base}/api/v1/hardware/devices`);
      if (res.ok) {
        const data = await res.json();
        setDevices(data.devices || []);
      }
    } catch {
      // Fallback local preset if backend is temporarily unreachable
    }
  }, []);

  useEffect(() => {
    fetchDevices();
  }, [fetchDevices]);

  // Connect device
  const handleConnect = async (deviceId: string) => {
    setLoadingDevice(deviceId);
    try {
      const base = getApiBaseUrl();
      const res = await fetch(`${base}/api/v1/hardware/${deviceId}/connect`, { method: "POST" });
      if (res.ok) {
        await fetchDevices();
        // Immediately fetch first telemetry
        fetchTelemetry(deviceId);
      }
    } catch (e: any) {
      console.error(e);
    } finally {
      setLoadingDevice(null);
    }
  };

  // Disconnect device
  const handleDisconnect = async (deviceId: string) => {
    setLoadingDevice(deviceId);
    try {
      const base = getApiBaseUrl();
      const res = await fetch(`${base}/api/v1/hardware/${deviceId}/disconnect`, { method: "POST" });
      if (res.ok) {
        await fetchDevices();
        setTelemetry(prev => {
          const next = { ...prev };
          delete next[deviceId];
          return next;
        });
      }
    } catch (e: any) {
      console.error(e);
    } finally {
      setLoadingDevice(null);
    }
  };

  // Read telemetry
  const fetchTelemetry = async (deviceId: string) => {
    try {
      const base = getApiBaseUrl();
      const res = await fetch(`${base}/api/v1/hardware/${deviceId}/telemetry`);
      if (res.ok) {
        const data = await res.json();
        setTelemetry(prev => ({ ...prev, [deviceId]: data }));
      } else if (res.status === 503) {
        const err = await res.json();
        setFaultAlert(`Fault injection observed on ${deviceId}: ${err.error} (Auto-recovery active)`);
        setTimeout(() => setFaultAlert(null), 4000);
      }
    } catch {
      // Suppress network jitter
    }
  };

  // Telemetry polling loop for connected devices
  useEffect(() => {
    if (!autoPoll) return;
    const interval = setInterval(() => {
      devices.forEach(d => {
        if (d.state === "CONNECTED") {
          fetchTelemetry(d.id);
        }
      });
    }, 1500);
    return () => clearInterval(interval);
  }, [devices, autoPoll]);

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
            Host-connected workbench architecture with three C++ mock adapters, simulated fault injection, and auto-recovery.
          </p>
        </div>

        <div className="flex items-center gap-3">
          <label className="flex items-center gap-2 text-xs text-gray-400 cursor-pointer bg-gray-900 px-3 py-1.5 rounded-lg border border-gray-800">
            <input 
              type="checkbox" 
              checked={autoPoll} 
              onChange={e => setAutoPoll(e.target.checked)}
              className="rounded bg-gray-800 border-gray-700 text-purple-600 focus:ring-0 cursor-pointer" 
            />
            <span>Auto-Poll Telemetry (1.5s)</span>
          </label>
        </div>
      </div>

      {/* Fault Injection Notice Banner */}
      {faultAlert && (
        <div className="p-3.5 rounded-xl bg-amber-950/70 border border-amber-800/80 text-xs text-amber-200 flex items-start gap-2.5 animate-pulse">
          <ShieldAlert className="w-4 h-4 text-amber-400 shrink-0 mt-0.5" />
          <div>
            <div className="font-bold">Simulated Fault Injection Event Detected:</div>
            <div className="font-mono text-[11px] text-amber-300/90">{faultAlert}</div>
          </div>
        </div>
      )}

      {/* 3 Device Cards */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-5">
        {devices.map((dev) => {
          const isConnected = dev.state === "CONNECTED";
          const telem = telemetry[dev.id];
          const isActing = loadingDevice === dev.id;

          return (
            <div 
              key={dev.id} 
              className={`p-5 rounded-xl bg-[#0f172a]/60 border transition-all flex flex-col justify-between space-y-4 ${
                isConnected ? "border-purple-600/50 shadow-lg shadow-purple-950/20" : "border-gray-800"
              }`}
            >
              <div>
                {/* Badges */}
                <div className="flex items-center justify-between mb-2">
                  <span className="text-[10px] font-mono font-bold px-2 py-0.5 rounded bg-purple-950/80 text-purple-300 border border-purple-800/60">
                    {dev.provenance || "SIMULATED"}
                  </span>
                  <span className={`text-[10px] font-mono font-bold px-2 py-0.5 rounded flex items-center gap-1 ${
                    isConnected 
                      ? "bg-emerald-950/70 text-emerald-400 border border-emerald-800/60" 
                      : "bg-gray-800 text-gray-400 border border-gray-700"
                  }`}>
                    <span className={`w-1.5 h-1.5 rounded-full ${isConnected ? "bg-emerald-400 animate-pulse" : "bg-gray-500"}`} />
                    <span>{dev.state}</span>
                  </span>
                </div>

                <h3 className="text-sm font-bold text-white">{dev.label}</h3>
                <div className="text-xs font-mono text-purple-400 mt-0.5">{dev.chip || dev.id}</div>
                <div className="text-[11px] text-gray-400 mt-1 font-mono">
                  {dev.interface || "Virtual COM / SWD / SPI"}
                </div>
              </div>

              {/* Live Telemetry Display */}
              {isConnected && telem ? (
                <div className="space-y-2 p-3 rounded-lg bg-gray-950/80 border border-gray-800 font-mono text-[11px]">
                  <div className="text-gray-400 text-[10px] uppercase font-bold tracking-wider pb-1 border-b border-gray-800 flex items-center justify-between">
                    <span>Live Telemetry</span>
                    <span className="text-emerald-400 flex items-center gap-1">
                      <Activity className="w-3 h-3" /> OK
                    </span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-gray-400">Voltage:</span>
                    <span className="text-blue-300 font-bold">{telem.supply_voltage_V.toFixed(2)} V</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-gray-400">Die Temp:</span>
                    <span className="text-amber-300 font-bold">{telem.core_temperature_C.toFixed(1)} °C</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-gray-400">Current:</span>
                    <span className="text-purple-300">{telem.current_mA.toFixed(1)} mA</span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-gray-400">GPIO Port:</span>
                    <span className="text-emerald-400">0x{telem.gpio_state.toString(16).toUpperCase().padStart(2, '0')}</span>
                  </div>
                  <div className="text-[9px] text-gray-500 truncate pt-1 border-t border-gray-900">
                    {telem.status_flags}
                  </div>
                </div>
              ) : (
                <div className="py-6 px-3 rounded-lg bg-gray-900/40 border border-gray-800/60 text-center text-xs text-gray-500 font-mono">
                  Device Offline — Click Connect
                </div>
              )}

              {/* Action Buttons */}
              <div className="pt-2">
                {isConnected ? (
                  <button 
                    onClick={() => handleDisconnect(dev.id)}
                    disabled={isActing}
                    className="w-full py-2 px-3 rounded-lg bg-red-950/80 hover:bg-red-900/80 border border-red-800/60 text-xs font-semibold text-red-300 transition-colors flex items-center justify-center gap-1.5 cursor-pointer"
                  >
                    <Power className="w-3.5 h-3.5" />
                    <span>Disconnect</span>
                  </button>
                ) : (
                  <button 
                    onClick={() => handleConnect(dev.id)}
                    disabled={isActing}
                    className="w-full py-2 px-3 rounded-lg bg-purple-600 hover:bg-purple-500 text-xs font-semibold text-white shadow-md shadow-purple-600/20 transition-all flex items-center justify-center gap-1.5 cursor-pointer"
                  >
                    <Power className="w-3.5 h-3.5" />
                    <span>Connect Device</span>
                  </button>
                )}
              </div>
            </div>
          );
        })}
      </div>

      {/* Interface Architecture & Pin-Voltage Isolation Matrix */}
      <div className="rounded-xl bg-[#0f172a]/60 p-6 border border-gray-800 space-y-4">
        <div className="flex items-center justify-between pb-3 border-b border-gray-800">
          <h3 className="text-sm font-bold text-white flex items-center gap-2">
            <Layers className="w-4 h-4 text-purple-400" />
            <span>Interface Architecture & Pin-Voltage Isolation Matrix</span>
          </h3>
          <span className="text-[11px] font-mono text-purple-400">
            Document: docs/architecture/embedded-system-design.md
          </span>
        </div>

        <div className="overflow-x-auto rounded-lg border border-gray-800 bg-gray-900/60">
          <table className="w-full text-left text-xs font-mono">
            <thead className="bg-gray-800/80 text-gray-300 text-[11px] uppercase tracking-wider border-b border-gray-700">
              <tr>
                <th className="px-3.5 py-2.5">Net Name</th>
                <th className="px-3.5 py-2.5">Source Device</th>
                <th className="px-3.5 py-2.5">Destination</th>
                <th className="px-3.5 py-2.5">Voltage</th>
                <th className="px-3.5 py-2.5">Direction</th>
                <th className="px-3.5 py-2.5">Function</th>
              </tr>
            </thead>
            <tbody className="divide-y divide-gray-800 text-gray-300 text-[11px]">
              <tr className="hover:bg-gray-800/40">
                <td className="px-3.5 py-2 text-white font-bold">USART2_TX</td>
                <td className="px-3.5 py-2 text-purple-300">STM32 PA2</td>
                <td className="px-3.5 py-2 text-gray-400">FT2232H RXD</td>
                <td className="px-3.5 py-2 text-blue-300">3.3 V</td>
                <td className="px-3.5 py-2">MCU → Host</td>
                <td className="px-3.5 py-2 text-gray-400">Debug/Telemetry UART</td>
              </tr>
              <tr className="hover:bg-gray-800/40">
                <td className="px-3.5 py-2 text-white font-bold">SWD_IO</td>
                <td className="px-3.5 py-2 text-purple-300">STM32 PA13</td>
                <td className="px-3.5 py-2 text-gray-400">FT2232H TMS</td>
                <td className="px-3.5 py-2 text-blue-300">3.3 V</td>
                <td className="px-3.5 py-2 text-amber-400 font-bold">Bidirectional</td>
                <td className="px-3.5 py-2 text-gray-400">ARM SWD Debug & Reset</td>
              </tr>
              <tr className="hover:bg-gray-800/40">
                <td className="px-3.5 py-2 text-white font-bold">SPI_MOSI</td>
                <td className="px-3.5 py-2 text-purple-300">STM32 PB5</td>
                <td className="px-3.5 py-2 text-purple-300">iCE40 SI</td>
                <td className="px-3.5 py-2 text-blue-300">3.3 V</td>
                <td className="px-3.5 py-2">MCU → FPGA</td>
                <td className="px-3.5 py-2 text-gray-400">Bitstream Configuration</td>
              </tr>
              <tr className="hover:bg-gray-800/40">
                <td className="px-3.5 py-2 text-white font-bold">VDD_1V2</td>
                <td className="px-3.5 py-2 text-gray-400">DC-DC Rail</td>
                <td className="px-3.5 py-2 text-purple-300">iCE40 Core</td>
                <td className="px-3.5 py-2 text-emerald-300 font-bold">1.2 V</td>
                <td className="px-3.5 py-2">Supply</td>
                <td className="px-3.5 py-2 text-gray-400">FPGA Core Logic Power</td>
              </tr>
              <tr className="hover:bg-gray-800/40">
                <td className="px-3.5 py-2 text-white font-bold">USB_VBUS</td>
                <td className="px-3.5 py-2 text-gray-400">Host USB 2.0</td>
                <td className="px-3.5 py-2 text-purple-300">FT2232H VBUS</td>
                <td className="px-3.5 py-2 text-amber-300 font-bold">5.0 V</td>
                <td className="px-3.5 py-2">Host → Probe</td>
                <td className="px-3.5 py-2 text-gray-400">USB 480 Mbps Interface Power</td>
              </tr>
            </tbody>
          </table>
        </div>

        <div className="flex items-center justify-between text-[11px] text-gray-400 font-mono pt-1">
          <span>Signal Isolation: Common ground reference shared across all 3 domains.</span>
          <span className="text-purple-400 font-bold">Provenance: 100% SIMULATED</span>
        </div>
      </div>
    </div>
  );
}
