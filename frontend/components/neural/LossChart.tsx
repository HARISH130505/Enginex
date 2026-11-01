"use client";

import React from "react";

interface StepData {
  step: number;
  loss: number;
  y_hat: number;
}

interface LossChartProps {
  history: StepData[];
}

export function LossChart({ history }: LossChartProps) {
  if (!history || history.length === 0) return null;

  const width = 700;
  const height = 300;
  const padLeft = 60;
  const padRight = 30;
  const padTop = 25;
  const padBottom = 40;

  const plotW = width - padLeft - padRight;
  const plotH = height - padTop - padBottom;

  const maxStep = history[history.length - 1].step;
  const maxLoss = Math.max(...history.map((h) => h.loss)) * 1.05 || 1.2;

  const toSvgX = (step: number) => padLeft + (step / maxStep) * plotW;
  const toSvgY = (loss: number) => padTop + (1 - loss / maxLoss) * plotH;

  const lossPath = history
    .map((h, i) => `${i === 0 ? "M" : "L"} ${toSvgX(h.step).toFixed(2)} ${toSvgY(h.loss).toFixed(2)}`)
    .join(" ");

  // Y-axis ticks
  const yTicks = [0, maxLoss * 0.25, maxLoss * 0.5, maxLoss * 0.75, maxLoss];
  // X-axis ticks (0, 20, 40, 60, 80, 100)
  const xTicks = [0, 20, 40, 60, 80, 100].filter((s) => s <= maxStep);

  return (
    <div className="w-full">
      <svg
        viewBox={`0 0 ${width} ${height}`}
        className="w-full h-auto"
        xmlns="http://www.w3.org/2000/svg"
        role="img"
        aria-label="Training loss curve"
      >
        {/* Background */}
        <rect x={padLeft} y={padTop} width={plotW} height={plotH} fill="#0d1117" rx="3" />

        {/* Grid lines */}
        {yTicks.map((y, i) => (
          <line
            key={`gy-${i}`}
            x1={padLeft}
            y1={toSvgY(y)}
            x2={padLeft + plotW}
            y2={toSvgY(y)}
            stroke="#1e293b"
            strokeWidth="1"
          />
        ))}

        {xTicks.map((x) => (
          <line
            key={`gx-${x}`}
            x1={toSvgX(x)}
            y1={padTop}
            x2={toSvgX(x)}
            y2={padTop + plotH}
            stroke="#1e293b"
            strokeWidth="1"
          />
        ))}

        {/* Axes */}
        <line x1={padLeft} y1={padTop + plotH} x2={padLeft + plotW} y2={padTop + plotH} stroke="#475569" strokeWidth="1" />
        <line x1={padLeft} y1={padTop} x2={padLeft} y2={padTop + plotH} stroke="#475569" strokeWidth="1" />

        {/* Y ticks & labels */}
        {yTicks.map((y, i) => (
          <text
            key={`yt-${i}`}
            x={padLeft - 8}
            y={toSvgY(y) + 3.5}
            textAnchor="end"
            fill="#94a3b8"
            fontSize="10"
            fontFamily="ui-monospace, monospace"
          >
            {y.toFixed(2)}
          </text>
        ))}

        {/* X ticks & labels */}
        {xTicks.map((x) => (
          <text
            key={`xt-${x}`}
            x={toSvgX(x)}
            y={padTop + plotH + 16}
            textAnchor="middle"
            fill="#94a3b8"
            fontSize="10"
            fontFamily="ui-monospace, monospace"
          >
            {x}
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
          Training Step (t)
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
          Squared Error Loss J(θ)
        </text>

        {/* Loss curve */}
        <path d={lossPath} fill="none" stroke="#ec4899" strokeWidth="2.5" />

        {/* Start and end points */}
        <circle cx={toSvgX(history[0].step)} cy={toSvgY(history[0].loss)} r="4.5" fill="#ec4899" />
        <circle cx={toSvgX(history[history.length - 1].step)} cy={toSvgY(history[history.length - 1].loss)} r="4.5" fill="#10b981" />

        {/* Legend */}
        <g>
          <rect
            x={padLeft + plotW - 190}
            y={padTop + 12}
            width={180}
            height={32}
            rx="4"
            fill="#0f172a"
            fillOpacity="0.85"
            stroke="#1e293b"
            strokeWidth="1"
          />
          <line x1={padLeft + plotW - 178} y1={padTop + 28} x2={padLeft + plotW - 150} y2={padTop + 28} stroke="#ec4899" strokeWidth="2.5" />
          <text x={padLeft + plotW - 144} y={padTop + 32} fill="#f472b6" fontSize="10" fontFamily="ui-monospace, monospace">
            Loss J = 0.5·(ŷ - y)²
          </text>
        </g>
      </svg>
    </div>
  );
}
