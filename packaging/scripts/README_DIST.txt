=================================================================
  ENGINEX Workbench — Desktop Engineering Platform
  PASS Science Works R&D Internship 2026-2027 Evaluation
=================================================================

QUICK START:
------------------------------------------------------
1. Double-click Launch.exe in this folder.
2. The launcher will automatically:
   - Identify an available ephemeral loopback TCP port (127.0.0.1).
   - Start the local C++ calculation server (bin\backend_server.exe).
   - Verify server health.
   - Open your default web browser to the Workbench interface.
3. To exit: Close the launcher console window or press ENTER / Ctrl+C.
   The launcher uses a Windows Job Object to manage the backend process lifetime;
   clean-machine shutdown behavior has not been tested.

TARGET AND VALIDATION LIMITS:
-----------------------------
- Intended target: Windows 11 x64.
- The package uses a local loopback service. Clean Windows 11, no-admin, and disconnected/offline-machine compatibility have NOT been tested.
- Node.js and Python are not needed to run the prebuilt frontend, but this is not a clean-machine compatibility claim.

PACKAGE CONTENTS:
-----------------
- Launch.exe                     : Win32 native turnkey launcher
- bin\backend_server.exe         : ISO C++17 REST simulation server
- Required compiler runtime DLLs are bundled beside each executable.
- web\                           : Statically exported Next.js client bundle
- data\geometry\sample_cube.stl  : Sample ASCII STL 3D geometry
- data\temperature\synthetic_ansys_sample.csv : Synthetic nodal-temperature fixture; not ANSYS output
- results\                       : Reference simulation & benchmark results

SUBMISSION SCOPE:
-----------------
This is an application runtime bundle, not a source submission. It does not include C++/frontend source, tests, CMake files, dependency lockfiles, editor configuration, project documentation, screenshots, or Git history/bundle. Check the separate submission checklist before submitting.

=================================================================
