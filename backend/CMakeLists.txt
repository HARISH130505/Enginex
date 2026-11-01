cmake_minimum_required(VERSION 3.20)
project(enginex_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
set(WORKBENCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 1. Core library target
add_library(workbench_core STATIC
    src/core/job_manager.cpp
)
target_include_directories(workbench_core PUBLIC
    ${WORKBENCH_INCLUDE_DIR}
)

# 2. Geometry library target
add_library(workbench_geometry STATIC
    src/geometry/stl_parser.cpp
    src/geometry/csv_parser.cpp
)
target_include_directories(workbench_geometry PUBLIC
    ${WORKBENCH_INCLUDE_DIR}
)
target_link_libraries(workbench_geometry PUBLIC
    workbench_core
)

# 3. Thermal FEA library target
add_library(workbench_thermal STATIC
    src/thermal/solver_1d.cpp
)
target_include_directories(workbench_thermal PUBLIC
    ${WORKBENCH_INCLUDE_DIR}
)
target_link_libraries(workbench_thermal PUBLIC
    workbench_core
)

# 4. API library target
add_library(workbench_api STATIC
    src/api/http_server.cpp
)
target_include_directories(workbench_api PUBLIC
    ${WORKBENCH_INCLUDE_DIR}
)
target_link_libraries(workbench_api PUBLIC
    workbench_geometry
    workbench_thermal
    workbench_core
)

if (WIN32)
    target_link_libraries(workbench_api PUBLIC ws2_32)
endif()

# 5. Backend server executable
add_executable(backend_server
    src/main.cpp
)
target_link_libraries(backend_server PRIVATE
    workbench_api
    workbench_geometry
    workbench_thermal
    workbench_core
)

if (WIN32)
    target_link_libraries(backend_server PRIVATE ws2_32)
endif()

# Set output directory to bin/
set_target_properties(backend_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
