cmake_minimum_required(VERSION 3.13)
project(OpenGL_learning)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
#
if(APPLE)
    #    set(CMAKE_CXX_FLAGS -L/usr/local/lib/ -lSDLmain -lSDL)
    include_directories(OpenGL_learning /usr/local/include/SDL2)
else()
    include_directories(${SDL2_INCLUDE_DIR})
endif()
link_directories(${SDL2_LIBRARY} ${OPENGL_gl_LIBRARY})
add_executable(OpenGL_learning main.cpp gl_core_4_3.c)
target_link_libraries(OpenGL_learning ${SDL2_LIBRARY} ${OPENGL_gl_LIBRARY})