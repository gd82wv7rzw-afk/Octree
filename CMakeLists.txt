cmake_minimum_required(VERSION 3.24)

set(APP_NAME barnesHut)
project(${APP_NAME} LANGUAGES CXX OBJCXX Swift)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-fsanitize=address>
        $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-g>
    )
    add_link_options(-sanitize=address)
endif()

set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_Swift_LANGUAGE_VERSION 5)

add_compile_options(
    "$<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-Wno-deprecated-declarations>"
    "$<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-Wno-availability>"
    "$<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-Wno-deprecated-anon-enum-enum-conversion>"
    "$<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-Wno-deprecated-enum-enum-conversion>"
)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Dependencies
include(FetchContent)

# Prüfe, ob metal_cpp bereits existiert
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/_deps/metal_cpp-src")
    set(METAL_CPP_OFFLINE ON)
else()
    set(METAL_CPP_OFFLINE OFF)
endif()

# Prüfe, ob metal_cpp_extensions bereits existiert
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/_deps/metal_cpp_extensions-src")
    set(METAL_EXT_OFFLINE ON)
else()
    set(METAL_EXT_OFFLINE OFF)
endif()

FetchContent_Declare(
    metal_cpp
    GIT_REPOSITORY https://github.com/apple/metal-cpp.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    UPDATE_DISCONNECTED ${METAL_CPP_OFFLINE}
    SOURCE_SUBDIR cmake-no-project
)

FetchContent_Declare(
    metal_cpp_extensions
    GIT_REPOSITORY https://github.com/olilarkin/metal-cpp-extensions.git
    GIT_TAG main
    GIT_SHALLOW TRUE
    UPDATE_DISCONNECTED ${METAL_EXT_OFFLINE}
    SOURCE_SUBDIR cmake-no-project
)

FetchContent_MakeAvailable(metal_cpp metal_cpp_extensions)

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
    "${SOURCE_DIR}/Gui/MainControler.mm"
)

# WICHTIG: RESOURCES muss hier mit übergeben werden!
add_executable(${APP_NAME} MACOSX_BUNDLE ${APP_SOURCES} ${RESOURCES})
set(SWIFT_BRIDGING_HEADER
    "${SOURCE_DIR}/Gui/Gui-Bridging-Header.h"
)

target_compile_options(${APP_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:Swift>:-import-objc-header>"
    "$<$<COMPILE_LANGUAGE:Swift>:${SWIFT_BRIDGING_HEADER}>"
)
target_compile_definitions(${APP_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX,OBJCXX>:DEFAULT_METALLIB_PATH=\"${METALLIB}\">"
)

# Automatisch den Projektnamen für das Menü und die App verwenden:
set_target_properties(${APP_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "${APP_NAME}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_NAME}"
    MACOSX_BUNDLE_INFO_STRING "${APP_NAME}"
    MACOSX_BUNDLE_PRINCIPAL_CLASS "NSApplication"
    #MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in"
)

# Metal library — jetzt direkt im App Bundle
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app/Contents/Resources")
set(METALLIB "${SHADER_OUTPUT_DIR}/default.metallib")

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

# Apple frameworks
set(APPLE_FRAMEWORKS
    SwiftUI
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

target_link_libraries(${APP_NAME} PRIVATE 
    ${APPLE_FRAMEWORK_PATHS}
    OpenMP::OpenMP_CXX
)
target_link_options(${APP_NAME} PRIVATE
    -Xlinker -no_warn_duplicate_libraries
)


# Metal shaders — paths relative to the project root
find_program(XCRUN xcrun REQUIRED)

set(SHADERS
    src/Simulation/Shader/calculateAccelleration.metal
    src/Simulation/Shader/firstIntegral.metal
    src/Simulation/Shader/updateCenterOfMass.metal
    src/Simulation/Shader/reduseMaxError.metal
    src/Simulation/Shader/findReinstertAtoms.metal
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