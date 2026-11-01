"use client";

import React, { useEffect, useRef, useState } from "react";
import * as THREE from "three";
// In modern Three.js OrbitControls can be imported from three/examples/jsm/controls/OrbitControls
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls.js";
import { Maximize2, Layers } from "lucide-react";

export interface StlTriangle {
  normal: { x: number; y: number; z: number };
  v1: { x: number; y: number; z: number };
  v2: { x: number; y: number; z: number };
  v3: { x: number; y: number; z: number };
}

interface StlViewerProps {
  triangles: StlTriangle[];
  wireframeDefault?: boolean;
}

export function StlViewer({ triangles, wireframeDefault = false }: StlViewerProps) {
  const containerRef = useRef<HTMLDivElement>(null);
  const [wireframe, setWireframe] = useState<boolean>(wireframeDefault);
  const controlsRef = useRef<OrbitControls | null>(null);
  const cameraRef = useRef<THREE.PerspectiveCamera | null>(null);
  const meshRef = useRef<THREE.Mesh | null>(null);

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

    const renderer = new THREE.WebGLRenderer({ antialias: true, alpha: false });
    renderer.setSize(width, height);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.shadowMap.enabled = true;
    container.innerHTML = "";
    container.appendChild(renderer.domElement);

    // 2. Controls
    const controls = new OrbitControls(camera, renderer.domElement);
    controls.enableDamping = true;
    controls.dampingFactor = 0.05;
    controlsRef.current = controls;

    // 3. Lighting
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.6);
    scene.add(ambientLight);

    const dirLight1 = new THREE.DirectionalLight(0xffffff, 0.8);
    dirLight1.position.set(5, 10, 7);
    scene.add(dirLight1);

    const dirLight2 = new THREE.DirectionalLight(0x38bdf8, 0.4);
    dirLight2.position.set(-5, -5, -5);
    scene.add(dirLight2);

    // 4. Grid and Axes
    const grid = new THREE.GridHelper(10, 20, 0x334155, 0x1e293b);
    grid.position.y = -0.001;
    scene.add(grid);

    // 5. Geometry from triangles
    if (triangles && triangles.length > 0) {
      const positions: number[] = [];
      const normals: number[] = [];

      for (const tri of triangles) {
        positions.push(tri.v1.x, tri.v1.y, tri.v1.z);
        positions.push(tri.v2.x, tri.v2.y, tri.v2.z);
        positions.push(tri.v3.x, tri.v3.y, tri.v3.z);

        // Add normals (or recompute vertex normals)
        normals.push(tri.normal.x, tri.normal.y, tri.normal.z);
        normals.push(tri.normal.x, tri.normal.y, tri.normal.z);
        normals.push(tri.normal.x, tri.normal.y, tri.normal.z);
      }

      const geom = new THREE.BufferGeometry();
      geom.setAttribute("position", new THREE.Float32BufferAttribute(positions, 3));
      geom.setAttribute("normal", new THREE.Float32BufferAttribute(normals, 3));
      geom.computeVertexNormals();
      geom.computeBoundingBox();

      const mat = new THREE.MeshStandardMaterial({
        color: 0x3b82f6,
        roughness: 0.4,
        metalness: 0.2,
        wireframe: wireframe,
        side: THREE.DoubleSide,
      });

      const mesh = new THREE.Mesh(geom, mat);
      meshRef.current = mesh;
      scene.add(mesh);

      // Fit camera to bounding box
      if (geom.boundingBox) {
        const center = geom.boundingBox.getCenter(new THREE.Vector3());
        const size = geom.boundingBox.getSize(new THREE.Vector3());
        const maxDim = Math.max(size.x, size.y, size.z) || 1.0;

        mesh.position.sub(center); // Center geometry at origin

        camera.position.set(maxDim * 1.8, maxDim * 1.5, maxDim * 2.2);
        camera.lookAt(0, 0, 0);
        controls.target.set(0, 0, 0);
        controls.update();
      }
    } else {
      camera.position.set(3, 3, 3);
      camera.lookAt(0, 0, 0);
    }

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
      renderer.dispose();
      if (container) container.innerHTML = "";
    };
  }, [triangles, wireframe]);

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
          onClick={() => setWireframe(!wireframe)}
          className={`px-2.5 py-1 rounded flex items-center gap-1.5 transition-colors ${
            wireframe ? "bg-blue-600 text-white" : "text-gray-300 hover:bg-gray-800"
          }`}
          title="Toggle Wireframe"
        >
          <Layers className="w-3.5 h-3.5" />
          <span>Wireframe</span>
        </button>
        <button
          onClick={handleFitToView}
          className="px-2.5 py-1 rounded text-gray-300 hover:bg-gray-800 flex items-center gap-1.5 transition-colors"
          title="Fit to View"
        >
          <Maximize2 className="w-3.5 h-3.5" />
          <span>Fit View</span>
        </button>
      </div>

      {/* Navigation Hint */}
      <div className="absolute bottom-2 left-3 text-[10px] text-gray-500 font-mono pointer-events-none bg-black/40 px-2 py-0.5 rounded">
        Orbit: Left Click • Pan: Right Click • Zoom: Scroll Wheel
      </div>
    </div>
  );
}
