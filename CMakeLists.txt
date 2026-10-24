
cmake_minimum_required(VERSION 3.24)

set(APP_NAME barnesHut)
project(${APP_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Dependencies
include(FetchContent)

FetchContent_Declare(
    metal_cpp
    GIT_REPOSITORY https://github.com/apple/metal-cpp.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR cmake-no-project
)

FetchContent_Declare(
    metal_cpp_extensions
    GIT_REPOSITORY https://github.com/olilarkin/metal-cpp-extensions.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR cmake-no-project
)

FetchContent_MakeAvailable(metal_cpp metal_cpp_extensions)

# Executable
set(APP_SOURCES
    "${SOURCE_DIR}/main.cpp"
    "${SOURCE_DIR}/MetalImplementation.cpp"
    "${SOURCE_DIR}/SimulatorV0.cpp"
    "${SOURCE_DIR}/MetalStuff/Pipeline/PipelineBuilder.cpp"
    "${SOURCE_DIR}/Simulation/MetalGravity.cpp"
)

add_executable(${APP_NAME} ${APP_SOURCES})

# Metal library — next to the executable
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(METALLIB "${SHADER_OUTPUT_DIR}/default.metallib")

target_compile_definitions(${APP_NAME} PRIVATE
    DEFAULT_METALLIB_PATH="${METALLIB}"
)

target_include_directories(${APP_NAME} PRIVATE
    "${SOURCE_DIR}"
    "${metal_cpp_SOURCE_DIR}"
    "${metal_cpp_extensions_SOURCE_DIR}"
)

# OpenMP (Homebrew)
find_program(BREW_EXECUTABLE brew REQUIRED)

execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix libomp
    OUTPUT_VARIABLE LIBOMP_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE BREW_RESULT
)

if(NOT BREW_RESULT EQUAL 0)
    message(FATAL_ERROR "Could not determine Homebrew libomp prefix.")
endif()

set(LIBOMP_INCLUDE_DIR "${LIBOMP_PREFIX}/include")
set(LIBOMP_LIBRARY "${LIBOMP_PREFIX}/lib/libomp.dylib")

if(NOT EXISTS "${LIBOMP_INCLUDE_DIR}/omp.h"
   OR NOT EXISTS "${LIBOMP_LIBRARY}")
    message(FATAL_ERROR
        "Homebrew libomp is missing. Install it with: brew install libomp"
    )
endif()

set(OpenMP_CXX_FLAGS
    "-Xpreprocessor -fopenmp -I${LIBOMP_INCLUDE_DIR}"
)
set(OpenMP_CXX_LIB_NAMES omp)
set(OpenMP_omp_LIBRARY "${LIBOMP_LIBRARY}")

find_package(OpenMP REQUIRED COMPONENTS CXX)

target_link_libraries(${APP_NAME} PRIVATE OpenMP::OpenMP_CXX)

# Apple frameworks
set(APPLE_FRAMEWORKS
    Foundation
    AppKit
    Metal
    MetalKit
    QuartzCore
)

set(APPLE_FRAMEWORK_PATHS)

foreach(FRAMEWORK IN LISTS APPLE_FRAMEWORKS)
    find_library(${FRAMEWORK}_FRAMEWORK NAMES ${FRAMEWORK} REQUIRED)
    list(APPEND APPLE_FRAMEWORK_PATHS "${${FRAMEWORK}_FRAMEWORK}")
endforeach()

target_link_libraries(${APP_NAME} PRIVATE ${APPLE_FRAMEWORK_PATHS})

# Metal shaders — paths relative to the project root
find_program(XCRUN xcrun REQUIRED)

set(SHADERS
    src/Octree/Implementation/BufferShader/copy.metal
)

set(AIR_FILES)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(SHADER_NAME "${SHADER}" NAME_WE)
    set(AIR_FILE "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.air")

    add_custom_command(
        OUTPUT "${AIR_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_OUTPUT_DIR}"
        COMMAND "${XCRUN}" -sdk macosx metal
                -c "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}"
                -o "${AIR_FILE}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}"
        VERBATIM
    )

    list(APPEND AIR_FILES "${AIR_FILE}")
endforeach()

add_custom_command(
    OUTPUT "${METALLIB}"
    COMMAND "${XCRUN}" -sdk macosx metallib
            ${AIR_FILES}
            -o "${METALLIB}"
    DEPENDS ${AIR_FILES}
    VERBATIM
)

add_custom_target(compile_shaders DEPENDS "${METALLIB}")
add_dependencies(${APP_NAME} compile_shaders)
