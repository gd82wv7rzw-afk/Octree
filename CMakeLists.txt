cmake_minimum_required(VERSION 3.24)

project(barnesHut
    VERSION 0.1.0
    LANGUAGES CXX OBJCXX Swift
)

set(APP_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_OBJCXX_STANDARD 20)
set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
set(CMAKE_OBJCXX_EXTENSIONS OFF)

set(CMAKE_Swift_LANGUAGE_VERSION 5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(SWIFT_BRIDGING_HEADER
    "${SOURCE_DIR}/Gui/Gui-Bridging-Header.h"
)

# Dependencies
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

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

FetchContent_MakeAvailable(
    metal_cpp
    metal_cpp_extensions
)

# Executable (Jetzt als MACOSX_BUNDLE, damit die GUI funktioniert)
set(APP_SOURCES
    #"${SOURCE_DIR}/main.cpp"
    "${SOURCE_DIR}/MetalImplementation.cpp"
    "${SOURCE_DIR}/SimulatorV0.cpp"
    "${SOURCE_DIR}/MetalStuff/Pipeline/PipelineBuilder.cpp"
    "${SOURCE_DIR}/Simulation/MetalGravity.cpp"
    "${SOURCE_DIR}/Gui/MyApp.swift"
    "${SOURCE_DIR}/Gui/ContentView.swift"
    "${SOURCE_DIR}/Gui/AppDelegate/AppDelegate.swift"
    "${SOURCE_DIR}/Gui/SidePanelView/SidePanelView.swift"
    "${SOURCE_DIR}/Gui/LogView/LogView.swift"
    "${SOURCE_DIR}/Gui/MetalView/MetalView.swift"
    "${SOURCE_DIR}/Gui/SettingsView/SettingsView.swift"
    "${SOURCE_DIR}/Gui/MainControler.mm"
    "${SOURCE_DIR}/Rendering/Renderer.cpp"
    "${SOURCE_DIR}/Rendering/Meshes/Mesh.cpp"
    "${SOURCE_DIR}/Rendering/Meshes/Icosphere.cpp"
    "${SOURCE_DIR}/Rendering/mtlm.cpp"
    "${SOURCE_DIR}/Rendering/TransformController.cpp"
    "${SOURCE_DIR}/MetalStuff/Pipeline/VertexDescriptorFactory.cpp"
    "${SOURCE_DIR}/Rendering/TripleBuffer.cpp"
    "${SOURCE_DIR}/Simulation/PostProzess.cpp"
)

add_executable(${APP_NAME} MACOSX_BUNDLE ${APP_SOURCES})

target_compile_options(${APP_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:Swift>:-import-objc-header>"
    "$<$<COMPILE_LANGUAGE:Swift>:${SWIFT_BRIDGING_HEADER}>"
)
target_compile_definitions(${APP_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX,OBJCXX>:DEFAULT_METALLIB_PATH=\"${METALLIB}\">"
)

# Configure macOS application bundle metadata
set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "de.eisbein.${APP_NAME}"
    MACOSX_BUNDLE_INFO_STRING "${APP_NAME}"
)

# Metal library output inside the application bundle
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app/Contents/Resources")
set(METALLIB "${SHADER_OUTPUT_DIR}/default.metallib")

# includes
target_include_directories(${APP_NAME} PRIVATE
    "${SOURCE_DIR}"
    "${metal_cpp_SOURCE_DIR}"
    "${metal_cpp_extensions_SOURCE_DIR}"
)

# OpenMP (Homebrew)
# Find OpenMP through Homebrew
find_program(BREW_EXECUTABLE brew REQUIRED)

execute_process(
    COMMAND "${BREW_EXECUTABLE}" --prefix libomp
    OUTPUT_VARIABLE LIBOMP_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

set(OpenMP_CXX_FLAGS
    "-Xpreprocessor -fopenmp -I${LIBOMP_PREFIX}/include"
)
set(OpenMP_CXX_LIB_NAMES omp)
set(OpenMP_omp_LIBRARY "${LIBOMP_PREFIX}/lib/libomp.dylib")

find_package(OpenMP REQUIRED COMPONENTS CXX)

# Link Apple system frameworks
target_link_libraries(${APP_NAME} PRIVATE
    "$<LINK_LIBRARY:FRAMEWORK,SwiftUI>"
    "$<LINK_LIBRARY:FRAMEWORK,Foundation>"
    "$<LINK_LIBRARY:FRAMEWORK,AppKit>"
    "$<LINK_LIBRARY:FRAMEWORK,Metal>"
    "$<LINK_LIBRARY:FRAMEWORK,MetalKit>"
    "$<LINK_LIBRARY:FRAMEWORK,QuartzCore>"
    OpenMP::OpenMP_CXX
)

# Metal shader compilation
find_program(XCRUN xcrun REQUIRED)

set(SHADERS
    "src/Simulation/Shader/calculateAcceleration.metal"
    "src/Simulation/Shader/firstIntegral.metal"
    "src/Simulation/Shader/updateCenterOfMass.metal"
    "src/Simulation/Shader/reduseMaxError.metal"
    "src/Simulation/Shader/findReinsertAtoms.metal"
    "src/Simulation/Shader/copyOutput.metal"
    "src/Rendering/Shader/render.metal"
)

file(GLOB_RECURSE METAL_HEADERS CONFIGURE_DEPENDS
    "${SOURCE_DIR}/*.h"
    "${SOURCE_DIR}/*.hpp"
)

set(AIR_FILES)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(SHADER_NAME "${SHADER}" NAME_WE)
    set(AIR_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.air")

    add_custom_command(
        OUTPUT "${AIR_FILE}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}"

        COMMAND "${XCRUN}" -sdk macosx metal
                -I "${SOURCE_DIR}"
                -c "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}"
                -o "${AIR_FILE}"

        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}"
                ${METAL_HEADERS}
        VERBATIM
    )

    list(APPEND AIR_FILES "${AIR_FILE}")
endforeach()

add_custom_command(
    OUTPUT "${METALLIB}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_OUTPUT_DIR}"
    COMMAND "${XCRUN}" -sdk macosx metallib
            ${AIR_FILES}
            -o "${METALLIB}"
    DEPENDS ${AIR_FILES}
    VERBATIM
)

add_custom_target(compile_shaders DEPENDS "${METALLIB}")
add_dependencies(${APP_NAME} compile_shaders)

# WICHTIG FÜR GUI: Hier sagen wir CMake, wie es .xib Dateien behandeln soll
#set_source_files_properties(${RESOURCES} PROPERTIES
#    MACOSX_PACKAGE_LOCATION "Resources"
#)