"use client";

import React from "react";
import Link from "next/link";
import { 
  Box, 
  Flame, 
  Cpu, 
  Radio, 
  Binary, 
  ArrowRight, 
  CheckCircle2, 
  Terminal, 
  Layers, 
  HardDrive 
} from "lucide-react";

const modules = [
  {
    id: "geometry",
    title: "Geometry & ANSYS Visualization",
    route: "/geometry/",
    icon: Box,
    badge: "Task B",
    accent: "from-blue-600/30 to-cyan-600/20",
    border: "border-blue-500/30",
    textAccent: "text-blue-400",
    desc: "Parse and inspect 3D ASCII STL triangle meshes with spatial bounding boxes. Visualize ANSYS nodal temperature CSV exports as interactive 3D colored point clouds with colormap legends.",
    stats: "STL Facet Parser • CSV Point Cloud • Three.js 3D Viewport"
  },
  {
    id: "thermal",
    title: "1D Steady-State Thermal FEA",
    route: "/thermal/",
    icon: Flame,
    badge: "Task C",
    accent: "from-amber-600/30 to-red-600/20",
    border: "border-amber-500/30",
    textAccent: "text-amber-400",
    desc: "Solve 1D steady-state heat conduction with uniform volumetric heat generation qA. Conduct convergence study for N = 10, 20, 40, 80 elements and verify midpoint errors against exact analytical solution.",
    stats: "Uniform 2-Node Elements • Dirichlet Elimination • Convergence Plots"
  },
  {
    id: "performance",
    title: "Multicore Execution Benchmark",
    route: "/performance/",
    icon: Cpu,
    badge: "Task D",
    accent: "from-emerald-600/30 to-teal-600/20",
    border: "border-emerald-500/30",
    textAccent: "text-emerald-400",
    desc: "Benchmark serial vs CPU-parallel thermal solve batches across 1–4 worker threads. Measure median speedup and efficiency over 5 repetitions with deterministic agreement (ΔT ≤ 10⁻⁸ °C).",
    stats: "Bounded Thread Pool • Deterministic Output • Monotonic Timings"
  },
  {
    id: "hardware",
    title: "Embedded System Integration",
    route: "/hardware/",
    icon: Radio,
    badge: "Task E",
    accent: "from-purple-600/30 to-violet-600/20",
    border: "border-purple-500/30",
    textAccent: "text-purple-400",
    desc: "Modular host-connected workbench architecture for Microcontroller (Cortex-M4), FPGA (Lattice iCE40), and JTAG Debug Adapter (FT2232H). C++ mock adapters with simulated fault injection & recovery.",
    stats: "Pin-Voltage Map • 3 C++ Mocks • Timeout & Retry Recovery"
  },
  {
    id: "backpropagation",
    title: "Neural Backpropagation",
    route: "/backpropagation/",
    icon: Binary,
    badge: "Task F",
    accent: "from-pink-600/30 to-rose-600/20",
    border: "border-pink-500/30",
    textAccent: "text-pink-400",
    desc: "Handcrafted 1-sample regression backpropagation in pure C++ without auto-diff. Analytical chain-rule gradients verified against central finite differences (ε = 10⁻⁶, error ≤ 10⁻⁵) and 100-step loss curve.",
    stats: "Handwritten Reference • Central FD Check • 100-Step Loss History"
  }
];

export default function DashboardPage() {
  return (
    <div className="space-y-8 pb-10">
      {/* Hero Section */}
      <div className="relative overflow-hidden rounded-2xl bg-gradient-to-r from-[#0d1627] via-[#101b30] to-[#0d1627] p-8 border border-gray-800 shadow-2xl">
        <div className="relative z-10 max-w-3xl">
          <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-blue-900/40 border border-blue-700/50 text-blue-300 text-xs font-medium mb-4">
            <span className="w-1.5 h-1.5 rounded-full bg-blue-400 animate-pulse"></span>
            PASS Science Works R&D Internship 2026–2027
          </div>
          <h1 className="text-3xl sm:text-4xl font-extrabold text-white tracking-tight leading-tight">
            ENGINEX <span className="text-transparent bg-clip-text bg-gradient-to-r from-blue-400 to-indigo-300">Workbench</span>
          </h1>
          <p className="mt-2 text-sm sm:text-base text-gray-300 font-light leading-relaxed">
            Unified Desktop Engineering Simulation & Analysis Platform. Statically hosted Next.js client paired with an ISO C++17 high-performance calculation core.
          </p>
          <div className="mt-6 flex flex-wrap gap-4 text-xs font-mono text-gray-400">
            <div className="flex items-center gap-1.5 bg-gray-900/80 px-3 py-1.5 rounded-md border border-gray-800">
              <CheckCircle2 className="w-3.5 h-3.5 text-emerald-400" />
              <span>C++17 Numerical Core</span>
            </div>
            <div className="flex items-center gap-1.5 bg-gray-900/80 px-3 py-1.5 rounded-md border border-gray-800">
              <HardDrive className="w-3.5 h-3.5 text-blue-400" />
              <span>Zero-Admin Turnkey Launch.exe</span>
            </div>
            <div className="flex items-center gap-1.5 bg-gray-900/80 px-3 py-1.5 rounded-md border border-gray-800">
              <Layers className="w-3.5 h-3.5 text-indigo-400" />
              <span>Offline Loopback (127.0.0.1)</span>
            </div>
          </div>
        </div>
      </div>

      {/* Grid of 5 Engineering Modules */}
      <div>
        <div className="flex items-center justify-between mb-4">
          <h2 className="text-lg font-bold text-white flex items-center gap-2">
            <span>Engineering Subsystems</span>
            <span className="text-xs font-normal text-gray-400 font-mono">(5 Integrated Views)</span>
          </h2>
        </div>

        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-5">
          {modules.map((mod) => {
            const Icon = mod.icon;
            return (
              <div
                key={mod.id}
                className={`flex flex-col justify-between rounded-xl bg-[#0f172a]/60 p-6 border ${mod.border} hover:border-gray-600 transition-all duration-200 group hover:shadow-lg hover:shadow-black/40`}
              >
                <div>
                  <div className="flex items-center justify-between mb-3">
                    <div className={`w-10 h-10 rounded-lg bg-gradient-to-br ${mod.accent} flex items-center justify-center border border-white/5`}>
                      <Icon className={`w-5 h-5 ${mod.textAccent}`} />
                    </div>
                    <span className="text-[11px] font-mono font-semibold px-2 py-0.5 rounded bg-gray-800/80 text-gray-300 border border-gray-700">
                      {mod.badge}
                    </span>
                  </div>
                  <h3 className="text-base font-bold text-white group-hover:text-blue-300 transition-colors">
                    {mod.title}
                  </h3>
                  <p className="mt-2 text-xs text-gray-400 leading-relaxed">
                    {mod.desc}
                  </p>
                </div>

                <div className="mt-5 pt-4 border-t border-gray-800/80">
                  <div className="text-[10px] font-mono text-gray-400 mb-3 truncate">
                    {mod.stats}
                  </div>
                  <Link
                    href={mod.route}
                    className={`inline-flex items-center gap-1.5 text-xs font-semibold ${mod.textAccent} hover:underline`}
                  >
                    <span>Launch Module</span>
                    <ArrowRight className="w-3.5 h-3.5 transition-transform group-hover:translate-x-1" />
                  </Link>
                </div>
              </div>
            );
          })}
        </div>
      </div>

      {/* System Architecture Specification Card */}
      <div className="rounded-xl bg-[#0c1220] p-6 border border-gray-800">
        <h3 className="text-sm font-bold text-white flex items-center gap-2 mb-3">
          <Terminal className="w-4 h-4 text-blue-400" />
          <span>System Topology & Verification Standards</span>
        </h3>
        <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-4 text-xs">
          <div className="p-3 rounded-lg bg-gray-900/60 border border-gray-800/80">
            <div className="font-semibold text-gray-300">Local Loopback Server</div>
            <div className="mt-1 text-gray-400 text-[11px]">Strictly bound to 127.0.0.1; ephemeral port selection to eliminate collisions.</div>
          </div>
          <div className="p-3 rounded-lg bg-gray-900/60 border border-gray-800/80">
            <div className="font-semibold text-gray-300">Pure C++17 Core</div>
            <div className="mt-1 text-gray-400 text-[11px]">All parsing, numerical FEA, multithreading, and neural algorithms in standard C++.</div>
          </div>
          <div className="p-3 rounded-lg bg-gray-900/60 border border-gray-800/80">
            <div className="font-semibold text-gray-300">Static Client Bundle</div>
            <div className="mt-1 text-gray-400 text-[11px]">Next.js static export served directly by C++ HTTP engine; zero Node.js at runtime.</div>
          </div>
          <div className="p-3 rounded-lg bg-gray-900/60 border border-gray-800/80">
            <div className="font-semibold text-gray-300">Windows Job Objects</div>
            <div className="mt-1 text-gray-400 text-[11px]">Launch.exe terminates all child backend processes automatically upon exit.</div>
          </div>
        </div>
      </div>
    </div>
  );
}

