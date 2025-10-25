cmake_minimum_required(VERSION 3.30)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd

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
    # fedora: mesa-libGLU
    set(GL_LIBS GL GLU glfw3 assimp freetype)
endif ()

set(SOURCES main.cpp src/extern/glad.c src/engine.hpp src/engine.cpp
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
        src/fonts.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${GL_LIBS})

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
