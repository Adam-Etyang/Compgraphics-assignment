cmake_minimum_required(VERSION 3.16)

project(ColourModelVisualizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(colour_visualizer main.cpp)
target_link_libraries(colour_visualizer PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)

include(CTest)
if(BUILD_TESTING)
    add_executable(color_math_tests color_math_tests.cpp)
    add_test(NAME color_conversion COMMAND color_math_tests)
endif()
