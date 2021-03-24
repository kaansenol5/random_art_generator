cmake_minimum_required(VERSION 3.19)


set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -lSDL2 -O4")
set(CMAKE_CXX_STANDARD 17)
option(USE_CLANG "build application with clang" ON)
project(randomart)

add_executable(
  randomart
  main.cpp

)
