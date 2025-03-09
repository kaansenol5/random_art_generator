cmake_minimum_required(VERSION 3.19)

project(randomart)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Find SDL2 package
find_package(SDL2 REQUIRED)
if(NOT SDL2_FOUND)
    # On macOS, try to find SDL2 via Homebrew if the standard find_package fails
    find_library(SDL2_LIBRARIES SDL2)
    if(NOT SDL2_LIBRARIES)
        message(FATAL_ERROR "SDL2 not found. Please install SDL2 using 'brew install sdl2'")
    endif()
    set(SDL2_INCLUDE_DIRS /usr/local/include/SDL2 /opt/homebrew/include/SDL2)
endif()

# Include SDL2 directories
include_directories(${SDL2_INCLUDE_DIRS})

# Add executable
add_executable(
    randomart
    main.cpp
)

# Link SDL2
target_link_libraries(randomart ${SDL2_LIBRARIES})

# Output binary to the root directory
set_target_properties(randomart PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
