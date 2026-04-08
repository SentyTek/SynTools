# ╒═════════════════════ CMakeLists.txt ═╕
# │ Syngine Tools                        │
# │ Created 2026-01-30                   │
# ├──────────────────────────────────────┤
# │ Copyright (c) SentyTek 2025-2026     │
# │ Licensed under the MIT License       │
# ╰──────────────────────────────────────╯
# SynTools CMake configuration

cmake_minimum_required(VERSION 3.16)
project(SynTools)
set(CMAKE_CXX_STANDARD 20)
set(SYNTOOLS_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src CACHE STRING "SynTools source dir" FORCE)

add_executable(syntools)

target_sources(syntools PRIVATE
    ${SYNTOOLS_SOURCE_DIR}/main.cpp
    ${SYNTOOLS_SOURCE_DIR}/ShaderCompiler.cpp
    ${SYNTOOLS_SOURCE_DIR}/AssetPackager.cpp
)

