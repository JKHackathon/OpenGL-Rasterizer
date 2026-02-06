cmake_minimum_required(VERSION 4.1)

project(Project1-HelloWorld LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(rasterizer main.cpp
                        ObjParser.cpp)

find_package(glfw3 3.4 REQUIRED)
target_link_libraries(rasterizer glfw)

find_package(GLEW REQUIRED)
target_link_libraries(rasterizer GLEW::GLEW)

find_package(OpenGL REQUIRED)
target_link_libraries(rasterizer OpenGL::GL)

target_compile_definitions(rasterizer PRIVATE GL_SILENCE_DEPRECATION)

find_package(glm REQUIRED)
target_link_libraries(rasterizer glm::glm)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
