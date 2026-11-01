cmake_minimum_required(VERSION 3.20)

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

set_target_properties(test_job_manager PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)

