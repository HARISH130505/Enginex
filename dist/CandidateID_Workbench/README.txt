=================================================================
  ENGINEX Workbench — Desktop Engineering Platform
  PASS Science Works R&D Internship 2026-2027 Evaluation
=================================================================

QUICK START (ZERO INSTALLATION / ZERO ADMIN REQUIRED):
------------------------------------------------------
1. Double-click Launch.exe in this folder.
2. The launcher will automatically:
   - Identify an available ephemeral loopback TCP port (127.0.0.1).
   - Start the local C++ calculation server (bin\backend_server.exe).
   - Verify server health.
   - Open your default web browser to the Workbench interface.
3. To exit: Close the launcher console window or press ENTER / Ctrl+C.
   Windows Job Objects guarantee all backend processes terminate cleanly.

REQUIREMENTS:
-------------
- Windows 11 x64 (or Windows 10 x64)
- Zero administrative privileges required
- Zero internet access required (operates 100% offline on loopback)
- No Python, MATLAB, Node.js, or compiler installation needed

PACKAGE CONTENTS:
-----------------
- Launch.exe                     : Win32 native turnkey launcher
- bin\backend_server.exe         : ISO C++17 REST simulation server
- Required compiler runtime DLLs are bundled beside each executable.
- web\                           : Statically exported Next.js client bundle
- data\geometry\sample_cube.stl  : Sample ASCII STL 3D geometry
- data\temperature\synthetic...  : Sample ANSYS nodal temperature CSV
- results\                       : Reference simulation & benchmark results

=================================================================
