cmake_minimum_required(VERSION 3.15)
project(mondev)

set(CMAKE_CXX_STANDARD 20)
include(FindPCAP.cmake)

add_executable(mondev main.cpp Sources/TapDevice.cpp Includes/TapDevice.h Includes/ILinuxDevice.h)
target_link_libraries(mondev ${PCAP_LIBRARY})