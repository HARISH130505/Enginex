"use client";

import React, { useEffect, useRef, useState } from "react";
import * as THREE from "three";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";
import { Maximize2, Info } from "lucide-react";

export interface TemperatureNode {
  node_id: number;
  x_m: number;
  y_m: number;
  z_m: number;
  temperature_C: number;
}

interface PointCloudViewerProps {
  nodes: TemperatureNode[];
  tempMin: number;
  tempMax: number;
}

// Convert normalized value [0, 1] to RGB thermal colormap
function getThermalColor(u: number): THREE.Color {
  const clamped = Math.max(0, Math.min(1, u));
  // 4-stop Jet-like spectrum: Blue -> Cyan -> Green -> Yellow -> Red
  let r = 0, g = 0, b = 0;
  if (clamped < 0.25) {
    const t = clamped / 0.25;
    r = 0;
    g = t;
    b = 1;
  } else if (clamped < 0.5) {
    const t = (clamped - 0.25) / 0.25;
    r = 0;
    g = 1;
    b = 1 - t;
  } else if (clamped < 0.75) {
    const t = (clamped - 0.5) / 0.25;
    r = t;
    g = 1;
    b = 0;
  } else {
    const t = (clamped - 0.75) / 0.25;
    r = 1;
    g = 1 - t;
    b = 0;
  }
  return new THREE.Color(r, g, b);
}

export function PointCloudViewer({ nodes, tempMin, tempMax }: PointCloudViewerProps) {
  const containerRef = useRef<HTMLDivElement>(null);
  const controlsRef = useRef<OrbitControls | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const [hoveredNode, setHoveredNode] = useState<TemperatureNode | null>(null);
  const [mousePos, setMousePos] = useState<{ x: number; y: number }>({ x: 0, y: 0 });

  useEffect(() => {
    if (!containerRef.current) return;
    const container = containerRef.current;
    const width = container.clientWidth || 600;
    const height = container.clientHeight || 450;

    // 1. Scene, Camera, Renderer
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0f1d);

    const camera = new THREE.PerspectiveCamera(45, width / height, 0.01, 1000);
    cameraRef.current = camera;

    const renderer = new THREE.WebGLRenderer({ antialias: true });
    renderer.setSize(width, height);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    container.innerHTML = "";
    container.appendChild(renderer.domElement);

    // 2. OrbitControls
    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.dampingFactor = 0.05;
    controlsRef.current = controls;

    // 3. Grid & Light
    const grid = new THREE.GridHelper(10, 20, 0x334155, 0x1e293b);
    grid.position.y = -0.001;
    scene.add(grid);

    // 4. Points Geometry
    let pointsMesh: THREE.Points | null = null;
    let centerVec = new THREE.Vector3(0, 0, 0);

    if (nodes && nodes.length > 0) {
      const positions: number[] = [];
      const colors: number[] = [];
      const deltaT = (tempMax - tempMin) > 1e-6 ? (tempMax - tempMin) : 1.0;

      const bbox = new THREE.Box3();
      for (const node of nodes) {
        positions.push(node.x_m, node.y_m, node.z_m);
        bbox.expandByPoint(new THREE.Vector3(node.x_m, node.y_m, node.z_m));

        const u = (node.temperature_C - tempMin) / deltaT;
        const color = getThermalColor(u);
        colors.push(color.r, color.g, color.b);
      }

      bbox.getCenter(centerVec);
      const sizeVec = bbox.getSize(new THREE.Vector3());
      const maxDim = Math.max(sizeVec.x, sizeVec.y, sizeVec.z) || 1.0;

      // Adjust positions to center around origin
      const adjustedPositions: number[] = [];
      for (let i = 0; i < positions.length; i += 3) {
        adjustedPositions.push(
          positions[i] - centerVec.x,
          positions[i + 1] - centerVec.y,
          positions[i + 2] - centerVec.z
        );
      }

      const geom = new THREE.BufferGeometry();
      geom.setAttribute("position", new THREE.Float32BufferAttribute(adjustedPositions, 3));
      geom.setAttribute("color", new THREE.Float32BufferAttribute(colors, 3));

      const mat = new THREE.PointsMaterial({
        size: 0.06,
        vertexColors: true,
        transparent: true,
        opacity: 0.95,
      });

      pointsMesh = new THREE.Points(geom, mat);
      scene.add(pointsMesh);

      camera.position.set(maxDim * 1.8, maxDim * 1.5, maxDim * 2.2);
      camera.lookAt(0, 0, 0);
      controls.target.set(0, 0, 0);
      controls.update();
    } else {
      camera.position.set(3, 3, 3);
      camera.lookAt(0, 0, 0);
    }

    // 5. Raycasting for Node Hover Inspection
    const raycaster = new THREE.Raycaster();
    raycaster.params.Points.threshold = 0.08;
    const mouse = new THREE.Vector2();

    const handleMouseMove = (event: MouseEvent) => {
      const rect = container.getBoundingClientRect();
      mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
      mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;
      setMousePos({ x: event.clientX - rect.left, y: event.clientY - rect.top });

      if (pointsMesh) {
        raycaster.setFromCamera(mouse, camera);
        const intersects = raycaster.intersectObject(pointsMesh);
        if (intersects.length > 0 && intersects[0].index !== undefined) {
          const idx = intersects[0].index;
          if (idx < nodes.length) {
            setHoveredNode(nodes[idx]);
            return;
          }
        }
      }
      setHoveredNode(null);
    };

    container.addEventListener("mousemove", handleMouseMove);

    // 6. Animation loop
    let animId: number;
    const animate = () => {
      animId = requestAnimationFrame(animate);
      controls.update();
      renderer.render(scene, camera);
    };
    animate();

    // 7. Resize handler
    const handleResize = () => {
      if (!container) return;
      const w = container.clientWidth;
      const h = container.clientHeight;
      camera.aspect = w / h;
      camera.updateProjectionMatrix();
      renderer.setSize(w, h);
    };
    window.addEventListener("resize", handleResize);

    return () => {
      cancelAnimationFrame(animId);
      window.removeEventListener("resize", handleResize);
      container.removeEventListener("mousemove", handleMouseMove);
      renderer.dispose();
      if (container) container.innerHTML = "";
    };
  }, [nodes, tempMin, tempMax]);

  const handleFitToView = () => {
    if (!controlsRef.current || !cameraRef.current) return;
    cameraRef.current.position.set(2.5, 2.0, 3.0);
    cameraRef.current.lookAt(0, 0, 0);
    controlsRef.current.target.set(0, 0, 0);
    controlsRef.current.update();
  };

  return (
    <div className="relative w-full h-full min-h-[420px] rounded-lg overflow-hidden bg-[#0a0f1d] border border-gray-800">
      <div ref={containerRef} className="w-full h-full min-h-[420px]" />

      {/* Control Overlay */}
      <div className="absolute top-3 right-3 flex items-center gap-2 bg-gray-900/80 backdrop-blur-md p-1.5 rounded-lg border border-gray-700 text-xs">
        <button
          onClick={handleFitToView}
          className="px-2.5 py-1 rounded text-gray-300 hover:bg-gray-800 flex items-center gap-1.5 transition-colors"
          title="Fit to View"
        >
          <Maximize2 className="w-3.5 h-3.5" />
          <span>Fit View</span>
        </button>
      </div>

      {/* Temperature Colormap Legend */}
      <div className="absolute bottom-4 right-4 bg-gray-900/90 backdrop-blur-md px-3 py-2 rounded-lg border border-gray-700 text-xs font-mono shadow-xl">
        <div className="text-[10px] text-gray-400 mb-1 flex justify-between">
          <span>Temperature Scale</span>
          <span>(°C)</span>
        </div>
        <div className="w-36 h-3 rounded bg-gradient-to-r from-blue-600 via-emerald-400 via-amber-400 to-red-600 border border-gray-600" />
        <div className="flex justify-between text-[10px] text-gray-300 mt-1">
          <span>{tempMin.toFixed(1)}°C</span>
          <span>{((tempMin + tempMax) * 0.5).toFixed(1)}°C</span>
          <span>{tempMax.toFixed(1)}°C</span>
        </div>
      </div>

      {/* Node Tooltip on Hover */}
      {hoveredNode && (
        <div
          className="absolute pointer-events-none z-20 bg-gray-950/90 backdrop-blur-md px-3 py-2 rounded-lg border border-blue-500/50 shadow-2xl text-xs font-mono text-gray-200"
          style={{
            left: Math.min(mousePos.x + 12, 380),
            top: Math.max(mousePos.y - 65, 10),
          }}
        >
          <div className="font-bold text-blue-400 flex items-center gap-1">
            <Info className="w-3 h-3" />
            <span>Node ID: {hoveredNode.node_id}</span>
          </div>
          <div className="text-[11px] text-gray-300 mt-0.5">
            Coord: ({hoveredNode.x_m.toFixed(3)}, {hoveredNode.y_m.toFixed(3)}, {hoveredNode.z_m.toFixed(3)}) m
          </div>
          <div className="text-[11px] font-semibold text-amber-300">
            Temp: {hoveredNode.temperature_C.toFixed(2)} °C
          </div>
        </div>
      )}

      {/* Navigation Hint */}
      <div className="absolute bottom-2 left-3 text-[10px] text-gray-500 font-mono pointer-events-none bg-black/40 px-2 py-0.5 rounded">
        Orbit: Left Click • Pan: Right Click • Zoom: Wheel • Hover on point to inspect
      </div>
    </div>
  );
}
