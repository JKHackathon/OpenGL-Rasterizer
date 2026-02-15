cmake_minimum_required(VERSION 4.1)

project(Project4-Textures LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(textures main.cpp
                        ../obj_loader.cpp
                        ../external/lodepng.cpp
                        ../rasterizer.cpp)

find_package(glfw3 3.4 REQUIRED)
target_link_libraries(textures glfw)

find_package(GLEW REQUIRED)
target_link_libraries(textures GLEW::GLEW)

find_package(OpenGL REQUIRED)
target_link_libraries(textures OpenGL::GL)

target_compile_definitions(textures PRIVATE GL_SILENCE_DEPRECATION)

find_package(glm REQUIRED)
target_link_libraries(textures glm::glm)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
