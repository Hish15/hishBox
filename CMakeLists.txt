cmake_minimum_required(VERSION 3.14)

# Set the project name and version
project(HishBox VERSION 1.0)


# Add CPM.cmake if it doesn't exist
if (NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
  file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake
       ${CMAKE_BINARY_DIR}/cmake/CPM.cmake)
endif()

# Include CPM
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

# Add dependencies


# Add the executable
add_executable(HishBox src/main.cpp)
