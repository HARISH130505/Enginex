import type { Config } from "tailwindcss";

const config: Config = {
  content: [
    "./pages/**/*.{js,ts,jsx,tsx,mdx}",
    "./components/**/*.{js,ts,jsx,tsx,mdx}",
    "./app/**/*.{js,ts,jsx,tsx,mdx}",
  ],
  theme: {
    extend: {
      colors: {
        background: "var(--background)",
        foreground: "var(--foreground)",
        workbench: {
          dark: "#0b0f19",
          panel: "#111827",
          border: "#1f2937",
          accent: "#2563eb",
          accentHover: "#1d4ed8",
          textMuted: "#9ca3af",
        }
      },
      fontFamily: {
        mono: ["Consolas", "Courier New", "monospace"],
      }
    },
  },
  plugins: [],
};
export default config;

