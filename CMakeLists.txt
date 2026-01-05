cmake_minimum_required(VERSION 3.30)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd

set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(TARGET_SHADER_COMPILER OFF CACHE BOOL "" FORCE)

set(Vulkan_GLSLC_EXECUTABLE "/usr/bin/true" CACHE FILEPATH "" FORCE)
set(Vulkan_DXC_EXECUTABLE "/usr/bin/true" CACHE FILEPATH "" FORCE)

include(FetchContent)
FetchContent_Declare(
  Jolt
  GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics.git
  GIT_TAG        master  # Or a specific version tag like v5.0.0
  SOURCE_SUBDIR Build
)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_MakeAvailable(Jolt)
# target_include_directories(${PROJECT_NAME} PRIVATE ${jolt_SOURCE_DIR})

set(GL_LIBS glfw3 assimp freetype)

if (CMAKE_SYSTEM MATCHES Windows)
    message(STATUS "Target system is Windows")
    # link/include for Windows x86
    include_directories(${CMAKE_SOURCE_DIR}/include/windows)
    link_directories(${CMAKE_SOURCE_DIR}/lib/windows)
elseif (CMAKE_SYSTEM MATCHES Linux)
    message(STATUS "Target system is linux")
    include_directories(${CMAKE_SOURCE_DIR}/include/linux
            ${CMAKE_SOURCE_DIR}/src/glad)
    # link_directories(lib)
    link_directories(${CMAKE_SOURCE_DIR}/lib/linux)
    # fedora: mesa-libGLU-devel
    set(GL_LIBS GL GLU glfw3 assimp freetype z pthread)
endif ()

set(SOURCES main.cpp src/extern/glad.c src/extern/mikktspace.c src/engine.hpp src/engine.cpp
        src/engine_types.hpp src/arena.hpp
        src/window.hpp src/window.cpp
        src/arena.cpp src/iohandler.hpp src/iohandler.cpp
        src/clock.hpp src/clock.cpp src/shader.cpp src/shader.hpp
        src/texture.hpp src/texture.cpp
        src/util.hpp
        src/shapes.hpp
        src/shapes.cpp
        src/camera.hpp
        src/mesh.cpp
        src/mesh.hpp
        src/model.hpp
        src/model.cpp
        src/postprocessing.cpp
        src/postprocessing.hpp
        src/timer.cpp
        src/fonts.hpp
        src/fonts.cpp
	src/ibl.hpp
	src/ibl.cpp
	src/bones.hpp
        src/bones.cpp
        src/renderer.cpp
        src/renderer.hpp
        src/entity.hpp
        src/entity.cpp
        src/scene.hpp
        src/scene.cpp
        src/bounds.hpp
        src/bounds.cpp
        src/physics.hpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE ${GL_LIBS} Jolt)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    JPH_DEBUG_RENDERER
    JPH_PROFILE_ENABLED
    JPH_EXTERNAL_PROFILE
    JPH_USE_SSE4_2
)

if (CMAKE_SYSTEM MATCHES Linux)
   target_compile_options(${PROJECT_NAME} PRIVATE -mpopcnt -msse4.2)
endif()

add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/data
        ${CMAKE_CURRENT_BINARY_DIR}/data
)
add_dependencies(${PROJECT_NAME} copy_assets)

add_custom_target(copy_shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_LIST_DIR}/src/shaders
        ${CMAKE_CURRENT_BINARY_DIR}/shaders
)
add_dependencies(${PROJECT_NAME} copy_assets)
add_dependencies(${PROJECT_NAME} copy_shaders)
