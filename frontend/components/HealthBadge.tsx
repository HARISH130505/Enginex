"use client";

import React, { useEffect, useState } from "react";
import { fetchHealth, getErrorMessage, type HealthResponse } from "@/lib/api/client";
import { Activity, AlertCircle } from "lucide-react";

export function HealthBadge() {
  const [health, setHealth] = useState<HealthResponse | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [loading, setLoading] = useState<boolean>(true);

  useEffect(() => {
    let active = true;
    const check = async () => {
      try {
        return { health: await fetchHealth(), error: null };
      } catch (err: unknown) {
        return { health: null, error: getErrorMessage(err, "Backend offline") };
      }
    };
    const refresh = () => {
      void check().then((result) => {
        if (!active) return;
        setHealth(result.health);
        setError(result.error);
        setLoading(false);
      });
    };
    refresh();
    const interval = setInterval(refresh, 3000);
    return () => {
      active = false;
      clearInterval(interval);
    };
  }, []);

  if (loading && !health && !error) {
    return (
      <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-gray-800/80 border border-gray-700 text-xs text-gray-400">
        <Activity className="w-3.5 h-3.5 animate-spin text-blue-400" />
        <span>Connecting C++ Core...</span>
      </div>
    );
  }

  if (error || !health) {
    return (
      <div className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-red-950/60 border border-red-800/60 text-xs text-red-300">
        <AlertCircle className="w-3.5 h-3.5 text-red-400" />
        <span>Core Offline ({error})</span>
      </div>
    );
  }

  return (
    <div className="flex items-center gap-2.5 px-3 py-1.5 rounded-full bg-emerald-950/50 border border-emerald-800/50 text-xs text-emerald-300">
      <span className="relative flex h-2 w-2">
        <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-emerald-400 opacity-75"></span>
        <span className="relative inline-flex rounded-full h-2 w-2 bg-emerald-500"></span>
      </span>
      <span className="font-medium">C++ Core v{health.version}</span>
      <span className="text-emerald-500/70 border-l border-emerald-800/60 pl-2 font-mono">
        Port {health.bound_port}
      </span>
    </div>
  );
}
