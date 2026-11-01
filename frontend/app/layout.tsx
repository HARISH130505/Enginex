import type { Metadata } from "next";
import "./globals.css";
import { Navbar } from "@/components/Navbar";

export const metadata: Metadata = {
  title: "ENGINEX Workbench — Unified Engineering Simulation & Analysis Platform",
  description: "PASS Science Works R&D Internship 2026–2027 Evaluation Project",
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en" className="dark">
      <body className="min-h-screen bg-[#070b14] text-gray-100 flex flex-col antialiased">
        <Navbar />
        <main className="flex-1 w-full max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-6">
          {children}
        </main>
        <footer className="border-t border-gray-800/80 bg-[#090e1a] py-4 text-center text-xs text-gray-500">
          <div className="max-w-7xl mx-auto px-4 flex flex-col sm:flex-row items-center justify-between gap-2">
            <span>ENGINEX Workbench • Windows 11 x64 Native Desktop Architecture</span>
            <span className="font-mono text-[11px] text-gray-400">Pure C++17 Core Engine • Offline Turnkey Deployment</span>
          </div>
        </footer>
      </body>
    </html>
  );
}

