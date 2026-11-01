"use client";

import React, { useState } from "react";
import { 
  Box, 
  Upload, 
  Eye, 
  FileText, 
  AlertTriangle, 
  Sparkles
} from "lucide-react";
import { StlViewer } from "@/components/geometry/StlViewer";
import type { StlTriangle } from "@/components/geometry/StlViewer";
import { PointCloudViewer } from "@/components/geometry/PointCloudViewer";
import type { TemperatureNode } from "@/components/geometry/PointCloudViewer";
import { getApiBaseUrl, getErrorMessage } from "@/lib/api/client";

interface StlInspectionResponse {
  error?: string;
  filename: string;
  triangle_count: number;
  bounding_box: { dimensions: { x: number; y: number; z: number } };
  triangles: StlTriangle[];
}

interface TemperatureDataResponse {
  error?: string;
  filename: string;
  node_count: number;
  temp_min: number;
  temp_max: number;
  temp_mean: number;
  provenance: string;
  nodes: TemperatureNode[];
}

export default function GeometryPage() {
  const [activeTab, setActiveTab] = useState<"stl" | "csv">("stl");

  // STL State
  const [stlData, setStlData] = useState<StlInspectionResponse | null>(null);
  const [stlLoading, setStlLoading] = useState<boolean>(false);
  const [stlError, setStlError] = useState<string | null>(null);

  // CSV State
  const [csvData, setCsvData] = useState<TemperatureDataResponse | null>(null);
  const [csvLoading, setCsvLoading] = useState<boolean>(false);
  const [csvError, setCsvError] = useState<string | null>(null);

  // Upload STL file to C++ Backend
  const handleStlUpload = async (file: File) => {
    setStlLoading(true);
    setStlError(null);
    try {
      const text = await file.text();
      const base = getApiBaseUrl();
      const res = await fetch(`${base}/api/v1/geometry/stl/inspect`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename: file.name, content: text }),
      });

      const data: StlInspectionResponse = await res.json();
      if (!res.ok) {
        throw new Error(data.error || "Failed to inspect STL file.");
      }
      setStlData(data);
      setActiveTab("stl");
    } catch (err: unknown) {
      setStlError(getErrorMessage(err, "An unexpected error occurred parsing STL."));
    } finally {
      setStlLoading(false);
    }
  };

  // Upload CSV file to C++ Backend
  const handleCsvUpload = async (file: File) => {
    setCsvLoading(true);
    setCsvError(null);
    try {
      const text = await file.text();
      const base = getApiBaseUrl();
      const res = await fetch(`${base}/api/v1/geometry/temperature/import`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename: file.name, content: text }),
      });

      const data: TemperatureDataResponse = await res.json();
      if (!res.ok) {
        throw new Error(data.error || "Failed to import CSV file.");
      }
      setCsvData(data);
      setActiveTab("csv");
    } catch (err: unknown) {
      setCsvError(getErrorMessage(err, "An unexpected error occurred parsing CSV."));
    } finally {
      setCsvLoading(false);
    }
  };

  // Preset Loaders using embedded sample fixtures
  const loadPresetStl = async () => {
    setStlLoading(true);
    setStlError(null);
    try {
      const sampleContent = `solid cube
  facet normal 0.0 0.0 1.0
    outer loop
      vertex 0.0 0.0 1.0
      vertex 1.0 0.0 1.0
      vertex 1.0 1.0 1.0
    endloop
  endfacet
  facet normal 0.0 0.0 1.0
    outer loop
      vertex 0.0 0.0 1.0
      vertex 1.0 1.0 1.0
      vertex 0.0 1.0 1.0
    endloop
  endfacet
  facet normal 0.0 0.0 -1.0
    outer loop
      vertex 0.0 0.0 0.0
      vertex 1.0 1.0 0.0
      vertex 1.0 0.0 0.0
    endloop
  endfacet
  facet normal 0.0 0.0 -1.0
    outer loop
      vertex 0.0 0.0 0.0
      vertex 0.0 1.0 0.0
      vertex 1.0 1.0 0.0
    endloop
  endfacet
  facet normal 0.0 1.0 0.0
    outer loop
      vertex 0.0 1.0 0.0
      vertex 0.0 1.0 1.0
      vertex 1.0 1.0 1.0
    endloop
  endfacet
  facet normal 0.0 1.0 0.0
    outer loop
      vertex 0.0 1.0 0.0
      vertex 1.0 1.0 1.0
      vertex 1.0 1.0 0.0
    endloop
  endfacet
  facet normal 0.0 -1.0 0.0
    outer loop
      vertex 0.0 0.0 0.0
      vertex 1.0 0.0 1.0
      vertex 0.0 0.0 1.0
    endloop
  endfacet
  facet normal 0.0 -1.0 0.0
    outer loop
      vertex 0.0 0.0 0.0
      vertex 1.0 0.0 0.0
      vertex 1.0 0.0 1.0
    endloop
  endfacet
  facet normal 1.0 0.0 0.0
    outer loop
      vertex 1.0 0.0 0.0
      vertex 1.0 1.0 1.0
      vertex 1.0 0.0 1.0
    endloop
  endfacet
  facet normal 1.0 0.0 0.0
    outer loop
      vertex 1.0 0.0 0.0
      vertex 1.0 1.0 0.0
      vertex 1.0 1.0 1.0
    endloop
  endfacet
  facet normal -1.0 0.0 0.0
    outer loop
      vertex 0.0 0.0 0.0
      vertex 0.0 0.0 1.0
      vertex 0.0 1.0 1.0
    endloop
  endfacet
  facet normal -1.0 0.0 0.0
    outer loop
      vertex 0.0 0.0 0.0
      vertex 0.0 1.0 1.0
      vertex 0.0 1.0 0.0
    endloop
  endfacet
endsolid cube`;
      const base = getApiBaseUrl();
      const res = await fetch(`${base}/api/v1/geometry/stl/inspect`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename: "sample_cube.stl", content: sampleContent }),
      });
      const data: StlInspectionResponse = await res.json();
      if (!res.ok) throw new Error(data.error);
      setStlData(data);
      setActiveTab("stl");
    } catch (err: unknown) {
      setStlError(getErrorMessage(err, "Failed to load sample STL."));
    } finally {
      setStlLoading(false);
    }
  };

  const loadPresetCsv = async () => {
    setCsvLoading(true);
    setCsvError(null);
    try {
      const sampleCsv = `node_id,x_m,y_m,z_m,temperature_C
1,0.00,0.00,0.00,100.0000
2,0.25,0.00,0.00,92.5000
3,0.50,0.00,0.00,81.0000
4,0.75,0.00,0.00,55.5000
5,1.00,0.00,0.00,20.0000
6,0.00,0.25,0.00,98.2000
7,0.25,0.25,0.00,90.4000
8,0.50,0.25,0.00,79.1000
9,0.75,0.25,0.00,53.8000
10,1.00,0.25,0.00,20.0000
11,0.00,0.50,0.00,95.0000
12,0.25,0.50,0.00,87.6000
13,0.50,0.50,0.00,76.5000
14,0.75,0.50,0.00,51.2000
15,1.00,0.50,0.00,20.0000
16,0.00,0.75,0.00,90.3000
17,0.25,0.75,0.00,83.1000
18,0.50,0.75,0.00,72.4000
19,0.75,0.75,0.00,47.9000
20,1.00,0.75,0.00,20.0000
21,0.00,1.00,0.00,85.0000
22,0.25,1.00,0.00,78.2000
23,0.50,1.00,0.00,67.5000
24,0.75,1.00,0.00,44.1000
25,1.00,1.00,0.00,20.0000
26,0.00,0.00,0.50,96.5000
27,0.25,0.00,0.50,89.3000
28,0.50,0.00,0.50,78.1000
29,0.75,0.00,0.50,53.2000
30,1.00,0.00,0.50,20.0000
31,0.00,0.50,0.50,92.1000
32,0.25,0.50,0.50,84.9000
33,0.50,0.50,0.50,74.0000
34,0.75,0.50,0.50,49.5000
35,1.00,0.50,0.50,20.0000
36,0.00,1.00,0.50,82.4000
37,0.25,1.00,0.50,75.8000
38,0.50,1.00,0.50,65.2000
39,0.75,1.00,0.50,42.7000
40,1.00,1.00,0.50,20.0000`;
      const base = getApiBaseUrl();
      const res = await fetch(`${base}/api/v1/geometry/temperature/import`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ filename: "synthetic_ansys_sample.csv", content: sampleCsv }),
      });
      const data: TemperatureDataResponse = await res.json();
      if (!res.ok) throw new Error(data.error);
      setCsvData(data);
      setActiveTab("csv");
    } catch (err: unknown) {
      setCsvError(getErrorMessage(err, "Failed to load sample CSV."));
    } finally {
      setCsvLoading(false);
    }
  };

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
            C++17 parsing of ASCII STL surface geometry and ANSYS nodal temperature distributions with Three.js 3D inspection.
          </p>
        </div>

        {/* Viewport Tab Selector */}
        <div className="flex items-center p-1 rounded-lg bg-gray-900 border border-gray-800 text-xs">
          <button
            onClick={() => setActiveTab("stl")}
            className={`flex items-center gap-2 px-3 py-1.5 rounded-md font-medium transition-colors ${
              activeTab === "stl" ? "bg-blue-600 text-white shadow" : "text-gray-400 hover:text-gray-200"
            }`}
          >
            <Box className="w-3.5 h-3.5" />
            <span>STL Surface Mesh</span>
          </button>
          <button
            onClick={() => setActiveTab("csv")}
            className={`flex items-center gap-2 px-3 py-1.5 rounded-md font-medium transition-colors ${
              activeTab === "csv" ? "bg-blue-600 text-white shadow" : "text-gray-400 hover:text-gray-200"
            }`}
          >
            <FileText className="w-3.5 h-3.5" />
            <span>ANSYS Temperature Cloud</span>
          </button>
        </div>
      </div>

      {/* Main Content Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left Column: Input Selectors and Metadata */}
        <div className="space-y-5">
          {/* Card 1: STL Upload */}
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
            <div className="flex items-center justify-between">
              <h2 className="text-xs font-bold text-white flex items-center gap-2">
                <Box className="w-4 h-4 text-blue-400" />
                <span>Input 1: ASCII STL Geometry</span>
              </h2>
              <button
                onClick={loadPresetStl}
                disabled={stlLoading}
                className="text-[11px] font-mono text-blue-400 hover:text-blue-300 flex items-center gap-1 bg-blue-950/60 px-2 py-1 rounded border border-blue-800/60 transition-colors"
                title="Load bundled sample cube STL fixture"
              >
                <Sparkles className="w-3 h-3" />
                <span>Load Sample</span>
              </button>
            </div>

            <label className="border-2 border-dashed border-gray-700 hover:border-blue-500/60 rounded-lg p-5 text-center cursor-pointer transition-colors bg-gray-900/40 block">
              <input
                type="file"
                accept=".stl"
                className="hidden"
                onChange={(e) => {
                  if (e.target.files && e.target.files[0]) {
                    handleStlUpload(e.target.files[0]);
                  }
                }}
              />
              <Upload className="w-7 h-7 text-gray-400 mx-auto mb-2" />
              <div className="text-xs font-medium text-gray-300">
                {stlLoading ? "Parsing C++ ASCII STL..." : "Drop ASCII STL file here or browse"}
              </div>
              <div className="text-[10px] text-gray-500 mt-1 font-mono">Format: ASCII (.stl) • Max: 50 MB</div>
            </label>

            {stlError && (
              <div className="p-3 rounded-lg bg-red-950/60 border border-red-800/80 text-xs text-red-300 flex items-start gap-2">
                <AlertTriangle className="w-4 h-4 text-red-400 shrink-0 mt-0.5" />
                <div className="leading-relaxed">{stlError}</div>
              </div>
            )}

            {stlData && (
              <div className="text-xs text-gray-300 bg-gray-900/80 p-3.5 rounded-lg border border-gray-800 space-y-1.5 font-mono">
                <div className="flex justify-between text-emerald-400 font-bold pb-1 border-b border-gray-800">
                  <span>File:</span> <span className="truncate max-w-[150px]">{stlData.filename}</span>
                </div>
                <div className="flex justify-between"><span>Triangles:</span> <span className="text-white font-bold">{stlData.triangle_count}</span></div>
                <div className="flex justify-between"><span>Dimensions:</span> <span>[{stlData.bounding_box.dimensions.x.toFixed(2)}, {stlData.bounding_box.dimensions.y.toFixed(2)}, {stlData.bounding_box.dimensions.z.toFixed(2)}]</span></div>
                <div className="flex justify-between text-[11px] text-gray-400"><span>Units Assumption:</span> <span>mm / m (dataset)</span></div>
              </div>
            )}
          </div>

          {/* Card 2: ANSYS CSV Upload */}
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
            <div className="flex items-center justify-between">
              <h2 className="text-xs font-bold text-white flex items-center gap-2">
                <FileText className="w-4 h-4 text-blue-400" />
                <span>Input 2: ANSYS Nodal CSV</span>
              </h2>
              <button
                onClick={loadPresetCsv}
                disabled={csvLoading}
                className="text-[11px] font-mono text-blue-400 hover:text-blue-300 flex items-center gap-1 bg-blue-950/60 px-2 py-1 rounded border border-blue-800/60 transition-colors"
                title="Load bundled synthetic ANSYS CSV fixture"
              >
                <Sparkles className="w-3 h-3" />
                <span>Load Synthetic</span>
              </button>
            </div>

            <label className="border-2 border-dashed border-gray-700 hover:border-blue-500/60 rounded-lg p-5 text-center cursor-pointer transition-colors bg-gray-900/40 block">
              <input
                type="file"
                accept=".csv,.txt"
                className="hidden"
                onChange={(e) => {
                  if (e.target.files && e.target.files[0]) {
                    handleCsvUpload(e.target.files[0]);
                  }
                }}
              />
              <Upload className="w-7 h-7 text-gray-400 mx-auto mb-2" />
              <div className="text-xs font-medium text-gray-300">
                {csvLoading ? "Parsing ANSYS Nodal CSV..." : "Drop ANSYS CSV file here or browse"}
              </div>
              <div className="text-[10px] text-gray-500 mt-1 font-mono">Required: node_id,x_m,y_m,z_m,temperature_C</div>
            </label>

            {csvError && (
              <div className="p-3 rounded-lg bg-red-950/60 border border-red-800/80 text-xs text-red-300 flex items-start gap-2">
                <AlertTriangle className="w-4 h-4 text-red-400 shrink-0 mt-0.5" />
                <div className="leading-relaxed">{csvError}</div>
              </div>
            )}

            {csvData && (
              <div className="text-xs text-gray-300 bg-gray-900/80 p-3.5 rounded-lg border border-gray-800 space-y-1.5 font-mono">
                <div className="flex justify-between text-emerald-400 font-bold pb-1 border-b border-gray-800">
                  <span>File:</span> <span className="truncate max-w-[150px]">{csvData.filename}</span>
                </div>
                <div className="flex justify-between"><span>Nodes:</span> <span className="text-white font-bold">{csvData.node_count}</span></div>
                <div className="flex justify-between"><span>Temp Range:</span> <span className="text-amber-300">{csvData.temp_min.toFixed(1)}°C to {csvData.temp_max.toFixed(1)}°C</span></div>
                <div className="flex justify-between"><span>Mean Temp:</span> <span>{csvData.temp_mean.toFixed(1)}°C</span></div>
                <div className="text-[10px] text-gray-400 pt-1 border-t border-gray-800 italic">
                  Provenance: {csvData.provenance}
                </div>
              </div>
            )}
          </div>
        </div>

        {/* Right Column: Interactive 3D Viewport */}
        <div className="lg:col-span-2 rounded-xl bg-[#0f172a]/60 border border-gray-800 flex flex-col justify-between overflow-hidden min-h-[520px]">
          {/* Top Viewport Header */}
          <div className="p-3.5 border-b border-gray-800 flex items-center justify-between bg-gray-900/60">
            <div className="flex items-center gap-2 text-xs font-semibold text-gray-200">
              <Eye className="w-4 h-4 text-blue-400" />
              <span>
                {activeTab === "stl" 
                  ? "3D ASCII STL Triangle Surface Mesh" 
                  : "3D ANSYS Nodal Temperature Point Cloud"}
              </span>
            </div>
            <div className="text-[11px] font-mono text-gray-400">
              Three.js WebGL Engine
            </div>
          </div>

          {/* Viewport Canvas Area */}
          <div className="flex-1 w-full h-full relative">
            {activeTab === "stl" ? (
              stlData && stlData.triangles ? (
                <StlViewer triangles={stlData.triangles} />
              ) : (
                <div className="flex flex-col items-center justify-center h-full min-h-[420px] p-8 text-center bg-[#070b14]/50">
                  <Box className="w-16 h-16 text-gray-700 mb-3 animate-pulse" />
                  <div className="text-sm font-semibold text-gray-300">No STL Geometry Loaded</div>
                  <p className="text-xs text-gray-500 max-w-sm mt-1">
                    Upload an ASCII STL file or click &quot;Load Sample&quot; to inspect triangles, normals, and bounding box.
                  </p>
                  <button
                    onClick={loadPresetStl}
                    className="mt-4 px-4 py-2 rounded-lg bg-blue-600 hover:bg-blue-500 text-xs font-medium text-white shadow-md shadow-blue-500/20 transition-all flex items-center gap-1.5"
                  >
                    <Sparkles className="w-3.5 h-3.5" />
                    <span>Load Bundled Sample Cube</span>
                  </button>
                </div>
              )
            ) : (
              csvData && csvData.nodes ? (
                <PointCloudViewer 
                  nodes={csvData.nodes} 
                  tempMin={csvData.temp_min} 
                  tempMax={csvData.temp_max} 
                />
              ) : (
                <div className="flex flex-col items-center justify-center h-full min-h-[420px] p-8 text-center bg-[#070b14]/50">
                  <FileText className="w-16 h-16 text-gray-700 mb-3 animate-pulse" />
                  <div className="text-sm font-semibold text-gray-300">No ANSYS Nodal Temperature Data Loaded</div>
                  <p className="text-xs text-gray-500 max-w-sm mt-1">
                    Upload an ANSYS CSV file or click &quot;Load Synthetic&quot; to inspect 3D colored nodes and thermal gradients.
                  </p>
                  <button
                    onClick={loadPresetCsv}
                    className="mt-4 px-4 py-2 rounded-lg bg-blue-600 hover:bg-blue-500 text-xs font-medium text-white shadow-md shadow-blue-500/20 transition-all flex items-center gap-1.5"
                  >
                    <Sparkles className="w-3.5 h-3.5" />
                    <span>Load Bundled Synthetic CSV</span>
                  </button>
                </div>
              )
            )}
          </div>

          {/* Bottom Bar: Dataset Assumptions & Limits */}
          <div className="p-3 border-t border-gray-800 bg-gray-900/60 flex flex-col sm:flex-row items-center justify-between text-[11px] text-gray-400 font-mono gap-2">
            <span>Mesh-to-Result Registration: Not required (independent viewers)</span>
            <span>File Limits: 50 MB / 500k triangles / 500k nodes</span>
          </div>
        </div>
      </div>
    </div>
  );
}
