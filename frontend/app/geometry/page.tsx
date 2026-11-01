"use client";

import React from "react";
import { Box, Upload, Eye, FileText, Info } from "lucide-react";

export default function GeometryPage() {
  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-4 border-b border-gray-800">
        <div>
          <div className="flex items-center gap-2">
            <h1 className="text-2xl font-bold text-white tracking-tight">CAD Geometry & ANSYS Visualization</h1>
            <span className="text-xs font-mono px-2 py-0.5 rounded bg-blue-900/60 text-blue-300 border border-blue-700/50">Task B</span>
          </div>
          <p className="text-xs text-gray-400 mt-1">
            ASCII STL triangle mesh inspection and ANSYS nodal temperature 3D colored point cloud visualization.
          </p>
        </div>
      </div>

      {/* Workflow Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left: Input Selection & File Validation */}
        <div className="space-y-4">
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
            <h2 className="text-sm font-bold text-white flex items-center gap-2">
              <Upload className="w-4 h-4 text-blue-400" />
              <span>Input 1: ASCII STL Geometry</span>
            </h2>
            <div className="border-2 border-dashed border-gray-700 hover:border-blue-500/60 rounded-lg p-6 text-center cursor-pointer transition-colors bg-gray-900/40">
              <Box className="w-8 h-8 text-gray-400 mx-auto mb-2" />
              <div className="text-xs font-medium text-gray-300">Drop ASCII STL file here, or click to browse</div>
              <div className="text-[11px] text-gray-500 mt-1 font-mono">Accepts: .stl (ASCII format only)</div>
            </div>
            <div className="text-xs text-gray-400 space-y-1 bg-gray-900/60 p-3 rounded-lg border border-gray-800">
              <div className="flex justify-between"><span>Status:</span> <span className="text-amber-400 font-mono">Awaiting file</span></div>
              <div className="flex justify-between"><span>Units:</span> <span className="text-gray-300 font-mono">Dataset assumption (mm/m)</span></div>
            </div>
          </div>

          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
            <h2 className="text-sm font-bold text-white flex items-center gap-2">
              <FileText className="w-4 h-4 text-blue-400" />
              <span>Input 2: ANSYS Nodal CSV Export</span>
            </h2>
            <div className="border-2 border-dashed border-gray-700 hover:border-blue-500/60 rounded-lg p-6 text-center cursor-pointer transition-colors bg-gray-900/40">
              <FileText className="w-8 h-8 text-gray-400 mx-auto mb-2" />
              <div className="text-xs font-medium text-gray-300">Drop ANSYS nodal CSV here</div>
              <div className="text-[11px] text-gray-500 mt-1 font-mono">Schema: node_id,x_m,y_m,z_m,temperature_C</div>
            </div>
            <div className="text-xs text-gray-400 space-y-1 bg-gray-900/60 p-3 rounded-lg border border-gray-800">
              <div className="flex justify-between"><span>Status:</span> <span className="text-amber-400 font-mono">Awaiting CSV</span></div>
              <div className="flex justify-between"><span>Provenance:</span> <span className="text-blue-300 font-mono">Synthetic Fixture</span></div>
            </div>
          </div>
        </div>

        {/* Right: 3D Viewport Placeholder */}
        <div className="lg:col-span-2 rounded-xl bg-[#0f172a]/60 border border-gray-800 flex flex-col justify-between overflow-hidden min-h-[480px]">
          <div className="p-4 border-b border-gray-800 flex items-center justify-between bg-gray-900/40">
            <div className="flex items-center gap-2 text-xs font-medium text-gray-300">
              <Eye className="w-4 h-4 text-blue-400" />
              <span>3D WebGL Viewport (Three.js OrbitControls)</span>
            </div>
            <div className="flex items-center gap-2 text-[11px] font-mono text-gray-500">
              <span>Orbit: L-Click • Pan: R-Click • Zoom: Wheel</span>
            </div>
          </div>

          <div className="flex-1 flex flex-col items-center justify-center p-8 text-center bg-[#070b14]/50">
            <Box className="w-16 h-16 text-gray-700 mb-3 animate-pulse" />
            <div className="text-sm font-semibold text-gray-400">3D Interactive Canvas Ready</div>
            <p className="text-xs text-gray-500 max-w-sm mt-1">
              Phase G2 will activate the live Three.js ASCII STL triangle renderer and 3D ANSYS colored point cloud inspector.
            </p>
          </div>

          <div className="p-3 border-t border-gray-800 bg-gray-900/60 flex items-center justify-between text-xs text-gray-400 font-mono">
            <span>Triangles: 0</span>
            <span>Nodes: 0</span>
            <span>Bounding Box: [0, 0, 0]</span>
          </div>
        </div>
      </div>
    </div>
  );
}

