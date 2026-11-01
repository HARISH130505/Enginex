"use client";

import React from "react";

interface WorkerResult {
  workers: number;
  median_ms: number;
  speedup: number;
  efficiency: number;
  max_temp_diff_vs_serial: number;
}

interface SpeedupChartProps {
  results: WorkerResult[];
}

export function SpeedupChart({ results }: SpeedupChartProps) {
  if (!results || results.length === 0) return null;

  const width = 700;
  const height = 300;
  const padLeft = 55;
  const padRight = 40;
  const padTop = 25;
  const padBottom = 40;

  const plotW = width - padLeft - padRight;
  const plotH = height - padTop - padBottom;

  const maxWorkers = Math.max(...results.map((r) => r.workers), 4);
  const maxSpeedup = Math.max(maxWorkers, ...results.map((r) => r.speedup)) * 1.1;

  const toSvgX = (w: number) => padLeft + ((w - 1) / (maxWorkers - 1)) * plotW;
  const toSvgY = (s: number) => padTop + (1 - s / maxSpeedup) * plotH;

  // Ideal scaling path (dashed gray)
  const idealPath = `M ${toSvgX(1)} ${toSvgY(1)} L ${toSvgX(maxWorkers)} ${toSvgY(maxWorkers)}`;

  // Measured speedup path
  const measuredPath = results
    .map((r, i) => `${i === 0 ? "M" : "L"} ${toSvgX(r.workers).toFixed(2)} ${toSvgY(r.speedup).toFixed(2)}`)
    .join(" ");

  // Y-axis ticks (0, 1, 2, 3, 4, ...)
  const yTicks: number[] = [];
  for (let s = 0; s <= Math.ceil(maxSpeedup); s += 1) {
    yTicks.push(s);
  }

  return (
    <div className="w-full">
      <svg
        viewBox={`0 0 ${width} ${height}`}
        className="w-full h-auto"
        xmlns="http://www.w3.org/2000/svg"
        role="img"
        aria-label="Speedup benchmark chart"
      >
        {/* Background */}
        <rect x={padLeft} y={padTop} width={plotW} height={plotH} fill="#0d1117" rx="3" />

        {/* Grid lines */}
        {yTicks.map((s) => (
          <line
            key={`gy-${s}`}
            x1={padLeft}
            y1={toSvgY(s)}
            x2={padLeft + plotW}
            y2={toSvgY(s)}
            stroke="#1e293b"
            strokeWidth="1"
          />
        ))}

        {results.map((r) => (
          <line
            key={`gx-${r.workers}`}
            x1={toSvgX(r.workers)}
            y1={padTop}
            x2={toSvgX(r.workers)}
            y2={padTop + plotH}
            stroke="#1e293b"
            strokeWidth="1"
          />
        ))}

        {/* Axes */}
        <line x1={padLeft} y1={padTop + plotH} x2={padLeft + plotW} y2={padTop + plotH} stroke="#475569" strokeWidth="1" />
        <line x1={padLeft} y1={padTop} x2={padLeft} y2={padTop + plotH} stroke="#475569" strokeWidth="1" />

        {/* Y ticks & labels */}
        {yTicks.map((s) => (
          <text
            key={`yt-${s}`}
            x={padLeft - 8}
            y={toSvgY(s) + 3.5}
            textAnchor="end"
            fill="#94a3b8"
            fontSize="10"
            fontFamily="ui-monospace, monospace"
          >
            {s.toFixed(1)}×
          </text>
        ))}

        {/* X ticks & labels */}
        {results.map((r) => (
          <text
            key={`xt-${r.workers}`}
            x={toSvgX(r.workers)}
            y={padTop + plotH + 16}
            textAnchor="middle"
            fill="#94a3b8"
            fontSize="10"
            fontFamily="ui-monospace, monospace"
          >
            {r.workers} {r.workers === 1 ? "worker" : "workers"}
          </text>
        ))}

        {/* Axis titles */}
        <text
          x={padLeft + plotW / 2}
          y={height - 5}
          textAnchor="middle"
          fill="#94a3b8"
          fontSize="11"
          fontFamily="sans-serif"
        >
          CPU Worker Count (p)
        </text>

        <text
          x={14}
          y={padTop + plotH / 2}
          textAnchor="middle"
          fill="#94a3b8"
          fontSize="11"
          fontFamily="sans-serif"
          transform={`rotate(-90, 14, ${padTop + plotH / 2})`}
        >
          Speedup Sp = T1 / Tp
        </text>

        {/* Ideal scaling line (dashed gray) */}
        <path d={idealPath} fill="none" stroke="#64748b" strokeWidth="1.5" strokeDasharray="5,4" />

        {/* Measured speedup line */}
        <path d={measuredPath} fill="none" stroke="#10b981" strokeWidth="2.5" />

        {/* Measured points and labels */}
        {results.map((r) => (
          <g key={`pt-${r.workers}`}>
            <circle cx={toSvgX(r.workers)} cy={toSvgY(r.speedup)} r="5" fill="#10b981" stroke="#0f172a" strokeWidth="2" />
            <text
              x={toSvgX(r.workers)}
              y={toSvgY(r.speedup) - 9}
              textAnchor="middle"
              fill="#34d399"
              fontSize="10"
              fontWeight="bold"
              fontFamily="ui-monospace, monospace"
            >
              {r.speedup.toFixed(2)}× ({r.median_ms.toFixed(1)} ms)
            </text>
          </g>
        ))}

        {/* Legend */}
        <g>
          <rect
            x={padLeft + 15}
            y={padTop + 12}
            width={180}
            height={50}
            rx="4"
            fill="#0f172a"
            fillOpacity="0.85"
            stroke="#1e293b"
            strokeWidth="1"
          />
          {/* Measured line */}
          <line x1={padLeft + 25} y1={padTop + 27} x2={padLeft + 50} y2={padTop + 27} stroke="#10b981" strokeWidth="2.5" />
          <circle cx={padLeft + 37.5} cy={padTop + 27} r="3.5" fill="#10b981" />
          <text x={padLeft + 56} y={padTop + 30} fill="#34d399" fontSize="10" fontFamily="ui-monospace, monospace">
            Measured Speedup Sp
          </text>

          {/* Ideal line */}
          <line x1={padLeft + 25} y1={padTop + 45} x2={padLeft + 50} y2={padTop + 45} stroke="#64748b" strokeWidth="1.5" strokeDasharray="4,3" />
          <text x={padLeft + 56} y={padTop + 48} fill="#94a3b8" fontSize="10" fontFamily="ui-monospace, monospace">
            Linear Ideal (Sp = p)
          </text>
        </g>
      </svg>
    </div>
  );
}

