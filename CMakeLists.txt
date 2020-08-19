cmake_minimum_required(VERSION 3.15)
project(mondevtopromisc)

set(CMAKE_CXX_STANDARD 20)

option(BUILD_DOC "Build doxygen" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    add_custom_target(doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)

include(FindPCAP.cmake)

add_executable(mondevtopromisc main.cpp
        Sources/Logger.cpp
        Sources/PCapReader.cpp
        Sources/TapDevice.cpp
        Sources/XLinkKaiConnection.cpp
        Includes/ILinuxDevice.h
        Includes/INetworkConnection.h
        Includes/IPCapDevice.h
        Includes/Logger.h
        Includes/PCapReader.h
        Includes/TapDevice.h
        Includes/XLinkKaiConnection.h)

target_link_libraries(mondevtopromisc ${PCAP_LIBRARY})