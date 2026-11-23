cmake_minimum_required(VERSION 3.16)

project(RocketFlight LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(rocket_movie main.cpp)
target_link_libraries(rocket_movie PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
