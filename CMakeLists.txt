cmake_minimum_required(VERSION 3.20)
project(ENGINEX VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common compiler warnings
if (MSVC)
    add_compile_options(/W4 /EHsc)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

enable_testing()

# Include subdirectories
add_subdirectory(backend)
add_subdirectory(tests)
add_subdirectory(packaging/launch)

