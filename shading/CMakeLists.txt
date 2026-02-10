cmake_minimum_required(VERSION 4.1)

project(Project3-Shading LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(shading main.cpp
                        ../obj_loader.cpp)

find_package(glfw3 3.4 REQUIRED)
target_link_libraries(shading glfw)

find_package(GLEW REQUIRED)
target_link_libraries(shading GLEW::GLEW)

find_package(OpenGL REQUIRED)
target_link_libraries(shading OpenGL::GL)

target_compile_definitions(shading PRIVATE GL_SILENCE_DEPRECATION)

find_package(glm REQUIRED)
target_link_libraries(shading glm::glm)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
