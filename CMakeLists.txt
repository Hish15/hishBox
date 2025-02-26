cmake_minimum_required(VERSION 3.14)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
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
CPMAddPackage("gh:gabime/spdlog@1.8.2")
CPMAddPackage("gh:fmtlib/fmt#7.1.3")
CPMAddPackage(
    NAME pn532
    GITHUB_REPOSITORY soonuse/pn532-lib
    GIT_TAG f25b40151f7940af89a75a95e11eb33e4b14581b
    DOWNLOAD_ONLY YES
)
CPMAddPackage("gh:SFML/SFML#2.6.2") #

if(pn532_ADDED)
add_library(pn532 STATIC  ${pn532_SOURCE_DIR}/pn532.c
                        ${pn532_SOURCE_DIR}/pn532_rpi.c)

target_include_directories(pn532  PUBLIC ${pn532_SOURCE_DIR})

endif()

# Add the executable
add_executable(HishBox src/main.cpp)
target_sources(HishBox PRIVATE
                    src/nfc_reader.cpp
				    src/nfc_reader.hpp
				    src/nfc_frame.hpp
                    src/media_player.cpp
                    src/media_player.hpp
	)
set_target_properties(HishBox PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES)
target_link_libraries(HishBox PRIVATE
                                        pn532
                                        spdlog
                                        fmt
                                        sfml-audio
                                        -lwiringPi)
target_include_directories(HishBox PRIVATE src)
