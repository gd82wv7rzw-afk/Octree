cmake_minimum_required(VERSION 3.20)

# ============================================================
# 🏷️ PROJECT CONFIGURATION
# ============================================================
set(APP_NAME barnesHut)

# Only C++ is required
project(${APP_NAME} LANGUAGES CXX)

# Export compile_commands.json (for clangd, tooling, etc.)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# ⚙️ LANGUAGE SETTINGS
# ============================================================
# Use modern C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# 🚀 TARGET DEFINITION
# ============================================================
add_executable(${APP_NAME}
    src/main.cpp
    src/Init.cpp
    src/SimulatorV0.cpp
)

# ============================================================
# 📦 INCLUDE DIRECTORIES
# ============================================================
target_include_directories(${APP_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# ============================================================
# 🔗 DEPENDENCIES & LINKING
# ============================================================
# OpenMP for parallelization (platform-independent)
find_package(OpenMP REQUIRED)

target_link_libraries(${APP_NAME}
    PRIVATE
        OpenMP::OpenMP_CXX
)