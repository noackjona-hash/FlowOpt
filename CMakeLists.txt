cmake_minimum_required(VERSION 3.16)
project(FlowOpt LANGUAGES CXX)

# --- Standard & Build-Typ ---------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Performance-Flags fuer den Headless-Core (Release).
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
option(FLOWOPT_NATIVE "Optimize for the host CPU (-march=native)" ON)
option(FLOWOPT_GUI    "Build the Raylib GUI application"          OFF)
option(FLOWOPT_TESTS  "Build unit tests"                          ON)

# --- Core-Bibliothek (keine Grafik-Abhaengigkeit) ---------------------------
add_library(flowopt_core STATIC
    src/graph/NetworkBuilder.cpp
    src/vehicle/VehiclePool.cpp
    src/routing/Router.cpp
    src/signals/FixedTimeController.cpp
    src/signals/RuleBasedController.cpp
    src/signals/PolicyController.cpp
    src/sim/Simulation.cpp
    src/io/ScenarioLoader.cpp
    src/io/Recorder.cpp
)
target_include_directories(flowopt_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(flowopt_core PRIVATE -Wall -Wextra)
if(FLOWOPT_NATIVE)
    target_compile_options(flowopt_core PUBLIC $<$<CONFIG:Release>:-march=native>)
endif()

# --- Headless-App (Standard) ------------------------------------------------
add_executable(flowopt_headless apps/main_headless.cpp)
target_include_directories(flowopt_headless PRIVATE ${CMAKE_SOURCE_DIR}/apps)
target_link_libraries(flowopt_headless PRIVATE flowopt_core)

# --- GUI-App (optional, benoetigt Raylib) -----------------------------------
if(FLOWOPT_GUI)
    find_package(raylib REQUIRED)
    add_executable(flowopt_gui apps/main_gui.cpp src/render/RaylibRenderer.cpp)
    target_include_directories(flowopt_gui PRIVATE ${CMAKE_SOURCE_DIR}/apps)
    target_link_libraries(flowopt_gui PRIVATE flowopt_core raylib)
endif()

# --- Tests ------------------------------------------------------------------
if(FLOWOPT_TESTS)
    enable_testing()
    add_executable(flowopt_tests tests/test_core.cpp)
    target_link_libraries(flowopt_tests PRIVATE flowopt_core)
    add_test(NAME core_smoke COMMAND flowopt_tests)
endif()
