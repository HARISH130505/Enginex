cmake_minimum_required(VERSION 3.20)

add_executable(Launch
    launch.cpp
)

if (WIN32)
    target_link_libraries(Launch PRIVATE ws2_32 shell32)
endif()

set_target_properties(Launch PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

