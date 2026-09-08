# forza_horizon_1 - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(forza_horizon_1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(FORZAHORIZON1_SOURCES
    src/main.cpp
    src/import_shims.cpp
)

if(WIN32)
    add_executable(forza_horizon_1 WIN32 ${FORZAHORIZON1_SOURCES})
else()
    add_executable(forza_horizon_1 ${FORZAHORIZON1_SOURCES})
endif()

rexglue_setup_target(forza_horizon_1)
