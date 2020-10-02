cmake_minimum_required(VERSION 3.15)
project(mondevtopromisc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_DOC "Build doxygen" OFF)
option(ENABLE_TESTS "Build unittests" OFF)

include_directories(Sources)
include_directories(Tests)

# check if Doxygen is installed
find_package(Doxygen)
if (BUILD_DOC AND DOXYGEN_FOUND)
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
endif (BUILD_DOC AND DOXYGEN_FOUND)

if (MSVC)
    message("You're running Windows! Make sure your paths are set correctly.")
    set(PCAP_ROOT_DIR "c:\\npcapsdk")
    set(BOOST_ROOT "c:\\Program Files\\boost\\boost_1_71_0")

    # Magic that boost on windows seems to need.
    add_definitions( -DBOOST_ALL_NO_LIB )
    add_definitions( -DBOOST_ALL_DYN_LINK )
endif ()

# Concepts seems to break on a bunch of compilers, see: https://github.com/boostorg/asio/issues/312
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

include(FindPCAP.cmake)

find_package(Boost 1.71 REQUIRED COMPONENTS thread system program_options)

find_package(Curses)

if (NOT APPLE AND NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif ()

add_executable(mondevtopromisc main.cpp
        Sources/Logger.cpp
        Sources/PacketConverter.cpp
        Sources/PCapReader.cpp
        Sources/WirelessMonitorDevice.cpp
        Sources/XLinkKaiConnection.cpp
        Sources/UserInterface/Button.cpp
        Sources/UserInterface/CheckBox.cpp
        Sources/UserInterface/NetworkingWindow.cpp
        Sources/UserInterface/String.cpp
        Sources/UserInterface/TextField.cpp
        Sources/UserInterface/UIObject.cpp
        Sources/UserInterface/Window.cpp
        Sources/UserInterface/WindowController.cpp
	Sources/WindowModel.cpp
        Sources/UserInterface/XLinkWindow.cpp
        Includes/IPCapDevice.h
        Includes/ISendReceiveDevice.h
        Includes/Logger.h
        Includes/NetworkingHeaders.h
        Includes/PacketConverter.h
        Includes/PCapReader.h
        Includes/WirelessMonitorDevice.h
        Includes/XLinkKaiConnection.h
        Includes/UserInterface/Button.h
        Includes/UserInterface/CheckBox.h
        Includes/UserInterface/IUIObject.h
        Includes/UserInterface/IWindow.h
        Includes/UserInterface/NetworkingWindow.h
        Includes/UserInterface/String.h
        Includes/UserInterface/TextField.h
        Includes/UserInterface/UIObject.h
        Includes/UserInterface/Window.h
        Includes/UserInterface/WindowController.h
	Includes/WindowModel.h
        Includes/UserInterface/XLinkWindow.h
        ${EXTRA_INCLUDES})

target_include_directories(mondevtopromisc PRIVATE ${PCAP_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${CURSES_INCLUDE_DIRS})
target_link_libraries(mondevtopromisc ${PCAP_LIBRARY} ${Boost_LIBRARIES} ${CURSES_LIBRARIES})

if (ENABLE_TESTS)
    find_package(GTest REQUIRED)
    include(GoogleTest)
    enable_testing()
    add_executable(tests Tests/PacketConverter_Test.cpp
            Sources/Logger.cpp
            Sources/PacketConverter.cpp
            Sources/PCapReader.cpp
            Sources/XLinkKaiConnection.cpp)
    target_include_directories(tests PRIVATE ${PCAP_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
    target_link_libraries(tests gtest gmock gtest_main ${PCAP_LIBRARY} ${Boost_LIBRARIES})
    gtest_discover_tests(tests)
endif(ENABLE_TESTS)
