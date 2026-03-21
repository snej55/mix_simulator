# On fedora: sudo dnf install mesa-libGLU-devel alsa-lib-devel
cmake_minimum_required(VERSION 3.30)
project(main)

file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.0/CPM.cmake ${CMAKE_BINARY_DIR}/cmake/CPM.cmake)
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

if (MSVC)
    message(STATUS "Using MSVC runtime DLLs")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set(USE_STATIC_MSVC_RUNTIME OFF CACHE BOOL "" FORCE)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd

set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(TARGET_SHADER_COMPILER OFF CACHE BOOL "" FORCE)
set(JPH_USE_VK OFF CACHE BOOL "" FORCE)
set(JPH_USE_DX12 OFF CACHE BOOL "" FORCE)
set(JPH_USE_MTL OFF CACHE BOOL "" FORCE)
set(JPH_USE_CPU_COMPUTE OFF CACHE BOOL "" FORCE)
set(GENERATE_SHADERS OFF CACHE BOOL "" FORCE)

set(JPH_USE_SSE4_2 ON CACHE BOOL "" FORCE)

set(JPH_DEBUG_RENDERER OFF CACHE BOOL "" FORCE)
set(JPH_PROFILE_ENABLED OFF CACHE BOOL "" FORCE)

include(FetchContent)

if (CMAKE_SYSTEM MATCHES Windows)
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
endif ()

FetchContent_Declare(
        Jolt
        GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
        GIT_TAG v5.5.0
        SOURCE_SUBDIR Build
)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.4
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(Jolt)
FetchContent_MakeAvailable(assimp)

find_package(Git QUIET)
if (GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    if (NOT EXISTS "${PROJECT_SOURCE_DIR}/external/soloud/include/soloud.h")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif ()
endif ()

set(SOLOUD_BACKEND_SDL2 OFF CACHE BOOL "" FORCE)
if (WIN32)
    set(SOLOUD_BACKEND_WINMM ON CACHE BOOL "" FORCE)
elseif (APPLE)
    set(SOLOUD_BACKEND_COREAUDIO ON CACHE BOOL "" FORCE)
else ()
    set(SOLOUD_BACKEND_ALSA ON CACHE BOOL "" FORCE)
    find_package(ALSA REQUIRED)
endif ()

add_subdirectory(external/soloud/contrib)

set(GL_LIBS glfw3 freetype)

if (CMAKE_SYSTEM MATCHES Windows)
    message(STATUS "Target system is Windows")
    # link/include for Windows x86
    include_directories(${CMAKE_SOURCE_DIR}/include/windows)
    link_directories(${CMAKE_SOURCE_DIR}/lib/windows)
elseif (CMAKE_SYSTEM MATCHES Linux)
    message(STATUS "Target system is linux")
    include_directories(${CMAKE_SOURCE_DIR}/include/linux
            ${CMAKE_SOURCE_DIR}/src/core/glad)
    # link_directories(lib)
    link_directories(${CMAKE_SOURCE_DIR}/lib/linux)
    # fedora: mesa-libGLU-devel
    set(GL_LIBS GL GLU glfw3 freetype z pthread)
endif ()

set(SOURCES main.cpp src/extern/glad.c src/extern/mikktspace.c src/core/engine.hpp src/core/engine.cpp
        src/core/engine_types.hpp src/core/arena.hpp
        src/core/window.hpp src/core/window.cpp
        src/core/arena.cpp src/core/iohandler.hpp src/core/iohandler.cpp
        src/core/clock.hpp src/core/clock.cpp src/core/shader.cpp src/core/shader.hpp
        src/core/texture.hpp src/core/texture.cpp
        src/core/camera.cpp
        src/core/engine.hpp
        src/core/util.hpp
        src/core/shapes.hpp
        src/core/shapes.cpp
        src/core/camera.hpp
        src/core/mesh.cpp
        src/core/mesh.hpp
        src/core/model.hpp
        src/core/model.cpp
        src/core/postprocessing.cpp
        src/core/postprocessing.hpp
        src/core/timer.cpp
        src/core/fonts.hpp
        src/core/fonts.cpp
        src/core/ibl.hpp
        src/core/ibl.cpp
        src/core/bones.hpp
        src/core/bones.cpp
        src/core/renderer.cpp
        src/core/renderer.hpp
        src/core/entity.hpp
        src/core/entity.cpp
        src/core/scene.hpp
        src/core/scene.cpp
        src/core/bounds.hpp
        src/core/bounds.cpp
        src/core/physics.hpp
        src/core/lights.hpp
        src/core/lights.cpp
        src/core/ui.hpp
        src/core/ui.cpp
        src/core/audio.hpp
        src/core/audio.cpp
        src/core/shadows.hpp
        src/core/shadows.cpp
        src/game.hpp
        src/game.cpp
        src/player.hpp
        src/player.cpp
        src/pathfinding.hpp
        src/pathfinding.cpp
        src/pathfinding.cpp
        src/enemy.hpp
        src/enemy.cpp
	src/shards.hpp
	src/shards.cpp
	src/particles.hpp
	src/particles.cpp
	src/dirt.hpp
	src/dirt.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${GL_LIBS} assimp Jolt soloud $<$<PLATFORM_ID:Linux>:${ALSA_LIBRARIES}>)

target_include_directories(${PROJECT_NAME} PRIVATE ${jolt_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/external/soloud/include)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        JPH_USE_SSE4_2
        $<$<PLATFORM_ID:Windows>:NOMINMAX>
)

if (CMAKE_SYSTEM MATCHES Linux)
    target_compile_options(${PROJECT_NAME} PRIVATE -mpopcnt -msse4.2)
endif ()

add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data
)

add_custom_target(copy_shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/src/shaders
        ${CMAKE_CURRENT_BINARY_DIR}/shaders
)
add_dependencies(${PROJECT_NAME} copy_assets)
add_dependencies(${PROJECT_NAME} copy_shaders)
