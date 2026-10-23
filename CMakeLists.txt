cmake_minimum_required(VERSION 3.20)

# ============================================================
# PROJECT CONFIGURATION
# ============================================================
set(APP_NAME barnesHut)

project(${APP_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

include(FetchContent)

FetchContent_Declare(
    metal_cpp
    GIT_REPOSITORY https://github.com/apple/metal-cpp.git
    GIT_TAG main
)

FetchContent_Declare(
    metal_cpp_extensions
    GIT_REPOSITORY https://github.com/olilarkin/metal-cpp-extensions.git
    GIT_TAG main
)

FetchContent_GetProperties(metal_cpp)
if(NOT metal_cpp_POPULATED)
    FetchContent_Populate(metal_cpp)
endif()

FetchContent_GetProperties(metal_cpp_extensions)
if(NOT metal_cpp_extensions_POPULATED)
    FetchContent_Populate(metal_cpp_extensions)
endif()

# ============================================================
# TARGET DEFINITION
# ============================================================
add_executable(${APP_NAME}
    src/main.cpp
    src/MetalImplementation.cpp
    src/SimulatorV0.cpp
)

# ============================================================
# INCLUDE DIRECTORIES
# ============================================================
target_include_directories(${APP_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${metal_cpp_SOURCE_DIR}"
        "${metal_cpp_extensions_SOURCE_DIR}"
)

# ============================================================
# OPENMP
# ============================================================
if(APPLE)
    find_program(BREW_EXECUTABLE brew REQUIRED)

    execute_process(
        COMMAND "${BREW_EXECUTABLE}" --prefix libomp
        OUTPUT_VARIABLE LIBOMP_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE BREW_RESULT
    )

    if(NOT BREW_RESULT EQUAL 0
       OR NOT EXISTS "${LIBOMP_PREFIX}/include/omp.h"
       OR NOT EXISTS "${LIBOMP_PREFIX}/lib/libomp.dylib")
        message(FATAL_ERROR
            "Homebrew libomp not found or incomplete. "
            "Install it with: brew install libomp"
        )
    endif()

    set(OpenMP_CXX_FLAGS
        "-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include"
    )
    set(OpenMP_CXX_LIB_NAMES omp)
    set(OpenMP_omp_LIBRARY "${LIBOMP_PREFIX}/lib/libomp.dylib")

    find_package(OpenMP REQUIRED COMPONENTS CXX)
else()
    find_package(OpenMP REQUIRED COMPONENTS CXX)
endif()

target_link_libraries(${APP_NAME}
    PRIVATE
        OpenMP::OpenMP_CXX
)

# ============================================================
# APPLE FRAMEWORKS
# ============================================================
if(APPLE)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)

    target_link_libraries(${APP_NAME}
        PRIVATE
            "${FOUNDATION_FRAMEWORK}"
            "${APPKIT_FRAMEWORK}"
            "${METAL_FRAMEWORK}"
            "${METALKIT_FRAMEWORK}"
            "${QUARTZCORE_FRAMEWORK}"
    )
endif()