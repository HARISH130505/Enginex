"use client";

import React, { useMemo } from "react";

interface ThermalChartProps {
  solutions: Array<{
    N: number;
    h: number;
    x_nodes: number[];
    T_fe: number[];
    T_exact: number[];
    max_midpoint_error: number;
  }>;
}

const MESH_COLORS: Record<number, string> = {
  10: "#f59e0b",
  20: "#3b82f6",
  40: "#10b981",
  80: "#8b5cf6",
};

const MESH_LABELS: Record<number, string> = {
  10: "FE N=10 (amber)",
  20: "FE N=20 (blue)",
  40: "FE N=40 (emerald)",
  80: "FE N=80 (violet)",
};

const PAD_LEFT = 60;
const PAD_BOTTOM = 40;
const PAD_TOP = 20;
const PAD_RIGHT = 20;
const SVG_WIDTH = 800;
const SVG_HEIGHT = 400;

const PLOT_W = SVG_WIDTH - PAD_LEFT - PAD_RIGHT;
const PLOT_H = SVG_HEIGHT - PAD_TOP - PAD_BOTTOM;

function niceInterval(range: number, targetTicks: number): number {
  const rough = range / targetTicks;
  const mag = Math.pow(10, Math.floor(Math.log10(rough)));
  const residual = rough / mag;
  let nice: number;
  if (residual <= 1.5) nice = 1;
  else if (residual <= 3) nice = 2;
  else if (residual <= 7) nice = 5;
  else nice = 10;
  return nice * mag;
}

export function ThermalChart({ solutions }: ThermalChartProps) {
  const { xMin, xMax, yMin, yMax, xTicks, yTicks, finestExact } = useMemo(() => {
    if (solutions.length === 0) {
      return { xMin: 0, xMax: 1, yMin: 0, yMax: 100, xTicks: [] as number[], yTicks: [] as number[], finestExact: null };
    }

    const xMn = 0;
    // Infer L from finest mesh
    const finest = solutions.reduce((a, b) => (a.N > b.N ? a : b));
    const xMx = finest.x_nodes[finest.x_nodes.length - 1];

    // Gather all temps
    let allTemps: number[] = [];
    for (const s of solutions) {
      allTemps = allTemps.concat(s.T_fe, s.T_exact);
    }
    const tMin = Math.min(...allTemps);
    const tMax = Math.max(...allTemps);
    const margin = (tMax - tMin) * 0.08 || 5;
    const yMn = Math.floor((tMin - margin) / 5) * 5;
    const yMx = Math.ceil((tMax + margin) / 5) * 5;

    // X ticks
    const xInterval = niceInterval(xMx - xMn, 5);
    const xT: number[] = [];
    for (let v = xMn; v <= xMx + xInterval * 0.001; v += xInterval) {
      xT.push(parseFloat(v.toFixed(10)));
    }

    // Y ticks
    const yInterval = niceInterval(yMx - yMn, 6);
    const yT: number[] = [];
    for (let v = yMn; v <= yMx + yInterval * 0.001; v += yInterval) {
      yT.push(parseFloat(v.toFixed(10)));
    }

    return {
      xMin: xMn,
      xMax: xMx,
      yMin: yMn,
      yMax: yMx,
      xTicks: xT,
      yTicks: yT,
      finestExact: { x: finest.x_nodes, T: finest.T_exact },
    };
  }, [solutions]);

  const toSvgX = (x: number) => PAD_LEFT + ((x - xMin) / (xMax - xMin)) * PLOT_W;
  const toSvgY = (y: number) => PAD_TOP + (1 - (y - yMin) / (yMax - yMin)) * PLOT_H;

  if (solutions.length === 0) return null;

  // Build exact solution path (dashed, white)
  const exactPath = finestExact
    ? finestExact.x
        .map((x, i) => `${i === 0 ? "M" : "L"}${toSvgX(x).toFixed(2)},${toSvgY(finestExact.T[i]).toFixed(2)}`)
        .join(" ")
    : "";

  // Build FE polylines
  const fePaths = solutions.map((s) => ({
    N: s.N,
    color: MESH_COLORS[s.N] || "#ffffff",
    path: s.x_nodes
      .map((x, i) => `${i === 0 ? "M" : "L"}${toSvgX(x).toFixed(2)},${toSvgY(s.T_fe[i]).toFixed(2)}`)
      .join(" "),
    points: s.x_nodes.map((x, i) => ({ cx: toSvgX(x), cy: toSvgY(s.T_fe[i]) })),
  }));

  return (
    <div className="w-full">
      <svg
        viewBox={`0 0 ${SVG_WIDTH} ${SVG_HEIGHT}`}
        className="w-full h-auto"
        xmlns="http://www.w3.org/2000/svg"
        role="img"
        aria-label="Temperature distribution chart"
      >
        {/* Chart background */}
        <rect x={PAD_LEFT} y={PAD_TOP} width={PLOT_W} height={PLOT_H} fill="#0d1117" rx="2" />

        {/* Grid lines */}
        {xTicks.map((v) => (
          <line
            key={`gx-${v}`}
            x1={toSvgX(v)}
            y1={PAD_TOP}
            x2={toSvgX(v)}
            y2={PAD_TOP + PLOT_H}
            stroke="#1e293b"
            strokeWidth="1"
          />
        ))}
        {yTicks.map((v) => (
          <line
            key={`gy-${v}`}
            x1={PAD_LEFT}
            y1={toSvgY(v)}
            x2={PAD_LEFT + PLOT_W}
            y2={toSvgY(v)}
            stroke="#1e293b"
            strokeWidth="1"
          />
        ))}

        {/* Axes */}
        <line x1={PAD_LEFT} y1={PAD_TOP + PLOT_H} x2={PAD_LEFT + PLOT_W} y2={PAD_TOP + PLOT_H} stroke="#475569" strokeWidth="1" />
        <line x1={PAD_LEFT} y1={PAD_TOP} x2={PAD_LEFT} y2={PAD_TOP + PLOT_H} stroke="#475569" strokeWidth="1" />

        {/* X tick labels */}
        {xTicks.map((v) => (
          <text
            key={`xt-${v}`}
            x={toSvgX(v)}
            y={PAD_TOP + PLOT_H + 16}
            textAnchor="middle"
            fill="#94a3b8"
            fontSize="10"
            fontFamily="ui-monospace, monospace"
          >
            {v.toFixed(1)}
          </text>
        ))}

        {/* Y tick labels */}
        {yTicks.map((v) => (
          <text
            key={`yt-${v}`}
            x={PAD_LEFT - 8}
            y={toSvgY(v) + 3.5}
            textAnchor="end"
            fill="#94a3b8"
            fontSize="10"
            fontFamily="ui-monospace, monospace"
          >
            {v.toFixed(0)}
          </text>
        ))}

        {/* X axis label */}
        <text
          x={PAD_LEFT + PLOT_W / 2}
          y={SVG_HEIGHT - 4}
          textAnchor="middle"
          fill="#94a3b8"
          fontSize="11"
          fontFamily="sans-serif"
        >
          Position x (m)
        </text>

        {/* Y axis label */}
        <text
          x={14}
          y={PAD_TOP + PLOT_H / 2}
          textAnchor="middle"
          fill="#94a3b8"
          fontSize="11"
          fontFamily="sans-serif"
          transform={`rotate(-90, 14, ${PAD_TOP + PLOT_H / 2})`}
        >
          Temperature T (°C)
        </text>

        {/* Exact solution (dashed white) */}
        {exactPath && (
          <path
            d={exactPath}
            fill="none"
            stroke="#e2e8f0"
            strokeWidth="1.5"
            strokeDasharray="6,3"
            opacity="0.85"
          />
        )}

        {/* FE solutions */}
        {fePaths.map((fp) => (
          <g key={`fe-${fp.N}`}>
            <path d={fp.path} fill="none" stroke={fp.color} strokeWidth="1.5" />
            {fp.points.map((p, i) => (
              <circle key={i} cx={p.cx} cy={p.cy} r="2.5" fill={fp.color} />
            ))}
          </g>
        ))}

        {/* Legend */}
        <g>
          <rect
            x={PAD_LEFT + PLOT_W - 165}
            y={PAD_TOP + 8}
            width={158}
            height={14 + solutions.length * 16 + 16}
            rx="4"
            fill="#0f172a"
            fillOpacity="0.85"
            stroke="#1e293b"
            strokeWidth="1"
          />
          {/* Exact legend entry */}
          <line
            x1={PAD_LEFT + PLOT_W - 155}
            y1={PAD_TOP + 24}
            x2={PAD_LEFT + PLOT_W - 140}
            y2={PAD_TOP + 24}
            stroke="#e2e8f0"
            strokeWidth="1.5"
            strokeDasharray="4,2"
          />
          <text
            x={PAD_LEFT + PLOT_W - 134}
            y={PAD_TOP + 28}
            fill="#e2e8f0"
            fontSize="9"
            fontFamily="ui-monospace, monospace"
          >
            Exact T(x)
          </text>

          {/* FE legend entries */}
          {solutions.map((s, idx) => {
            const yLeg = PAD_TOP + 24 + (idx + 1) * 16;
            const color = MESH_COLORS[s.N] || "#ffffff";
            return (
              <g key={`leg-${s.N}`}>
                <circle cx={PAD_LEFT + PLOT_W - 148} cy={yLeg} r="4" fill={color} />
                <text
                  x={PAD_LEFT + PLOT_W - 134}
                  y={yLeg + 3.5}
                  fill="#cbd5e1"
                  fontSize="9"
                  fontFamily="ui-monospace, monospace"
                >
                  {`FE N=${s.N} (h=${s.h.toFixed(4)})`}
                </text>
              </g>
            );
          })}
        </g>
      </svg>
    </div>
  );
}

