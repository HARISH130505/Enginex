cmake_minimum_required(VERSION 3.20)

# 1. Core JobManager test
add_executable(test_job_manager
    unit/test_job_manager.cpp
)
target_include_directories(test_job_manager PRIVATE
    ${CMAKE_SOURCE_DIR}/backend/include
)
target_link_libraries(test_job_manager PRIVATE
    workbench_core
)
add_test(NAME JobManagerLifecycleTest COMMAND test_job_manager)
set_tests_properties(JobManagerLifecycleTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_job_manager PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

# 2. ASCII STL Parser test
add_executable(test_stl_parser
    unit/test_stl_parser.cpp
)
target_include_directories(test_stl_parser PRIVATE
    ${CMAKE_SOURCE_DIR}/backend/include
)
target_link_libraries(test_stl_parser PRIVATE
    workbench_geometry
    workbench_core
)
add_test(NAME StlParserTest COMMAND test_stl_parser)
set_tests_properties(StlParserTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_stl_parser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

# 3. ANSYS CSV Parser test
add_executable(test_csv_parser
    unit/test_csv_parser.cpp
)
target_include_directories(test_csv_parser PRIVATE
    ${CMAKE_SOURCE_DIR}/backend/include
)
target_link_libraries(test_csv_parser PRIVATE
    workbench_geometry
    workbench_core
)
add_test(NAME CsvParserTest COMMAND test_csv_parser)
set_tests_properties(CsvParserTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_csv_parser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

# 4. Thermal FEA Solver test
add_executable(test_thermal_solver
    unit/test_thermal_solver.cpp
)
target_include_directories(test_thermal_solver PRIVATE
    ${CMAKE_SOURCE_DIR}/backend/include
)
target_link_libraries(test_thermal_solver PRIVATE
    workbench_thermal
    workbench_core
)
add_test(NAME ThermalSolverTest COMMAND test_thermal_solver)
set_tests_properties(ThermalSolverTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_thermal_solver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

# 5. Batch Generator test
add_executable(test_batch_generator
    unit/test_batch_generator.cpp
)
target_include_directories(test_batch_generator PRIVATE
    ${CMAKE_SOURCE_DIR}/backend/include
)
target_link_libraries(test_batch_generator PRIVATE
    workbench_performance
    workbench_thermal
    workbench_core
)
add_test(NAME BatchGeneratorTest COMMAND test_batch_generator)
set_tests_properties(BatchGeneratorTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_batch_generator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

# 6. Concurrency & Agreement test
add_executable(test_concurrency_agreement
    unit/test_concurrency_agreement.cpp
)
target_include_directories(test_concurrency_agreement PRIVATE
    ${CMAKE_SOURCE_DIR}/backend/include
)
target_link_libraries(test_concurrency_agreement PRIVATE
    workbench_performance
    workbench_thermal
    workbench_core
)
add_test(NAME ConcurrencyAgreementTest COMMAND test_concurrency_agreement)
set_tests_properties(ConcurrencyAgreementTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_concurrency_agreement PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

# 7. Mock Hardware Adapter test
add_executable(test_mock_hardware
    unit/test_mock_hardware.cpp
)
target_include_directories(test_mock_hardware PRIVATE
    ${CMAKE_SOURCE_DIR}/backend/include
)
target_link_libraries(test_mock_hardware PRIVATE
    workbench_hardware
    workbench_core
)
add_test(NAME MockHardwareTest COMMAND test_mock_hardware)
set_tests_properties(MockHardwareTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(test_mock_hardware PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
