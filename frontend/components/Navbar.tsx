"use client";

import React from "react";
import Link from "next/link";
import { usePathname } from "next/navigation";
import { 
  Box, 
  Flame, 
  Cpu, 
  Radio, 
  Binary, 
  Home
} from "lucide-react";
import { HealthBadge } from "./HealthBadge";

const navItems = [
  { href: "/", label: "Dashboard", icon: Home },
  { href: "/geometry/", label: "Geometry", icon: Box, subtitle: "CAD & ANSYS" },
  { href: "/thermal/", label: "Thermal Analysis", icon: Flame, subtitle: "1D FEA" },
  { href: "/performance/", label: "Performance", icon: Cpu, subtitle: "Multicore" },
  { href: "/hardware/", label: "Hardware", icon: Radio, subtitle: "Simulated Peripherals" },
  { href: "/backpropagation/", label: "Backpropagation", icon: Binary, subtitle: "Handcrafted Neural" },
];

export function Navbar() {
  const pathname = usePathname();

  return (
    <header className="sticky top-0 z-50 bg-[#0c1222]/90 backdrop-blur-md border-b border-gray-800">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16">
          {/* Brand */}
          <div className="flex items-center gap-3">
            <Link href="/" prefetch={false} className="flex items-center gap-3 group">
              <div>
                <div className="text-base font-bold text-white tracking-tight flex items-center gap-2">
                  ENGINEX <span className="text-xs font-semibold px-1.5 py-0.5 rounded bg-blue-900/60 text-blue-300 border border-blue-700/50">Workbench</span>
                </div>
                <div className="text-[10px] text-gray-400 font-mono tracking-wider uppercase">
                  Simulation & Analysis Platform
                </div>
              </div>
            </Link>
          </div>

          {/* Navigation Links */}
          <nav className="hidden md:flex items-center space-x-1">
            {navItems.map((item) => {
              const Icon = item.icon;
              const isActive = pathname === item.href || (item.href !== "/" && pathname.startsWith(item.href));
              return (
                <Link
                  key={item.href}
                  href={item.href}
                  prefetch={false}
                  className={`flex items-center gap-2 px-3 py-2 rounded-md text-xs font-medium transition-all ${
                    isActive
                      ? "bg-blue-600/20 text-blue-400 border border-blue-500/30"
                      : "text-gray-400 hover:text-gray-200 hover:bg-gray-800/60"
                  }`}
                >
                  <Icon className={`w-4 h-4 ${isActive ? "text-blue-400" : "text-gray-400"}`} />
                  <span>{item.label}</span>
                </Link>
              );
            })}
          </nav>

          {/* Right Area: Health & System Status */}
          <div className="flex items-center gap-3">
            <HealthBadge />
          </div>
        </div>
      </div>
    </header>
  );
}
