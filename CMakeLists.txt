cmake_minimum_required(VERSION 3.15)
project(xlinkhandheldassistant)

# TODO fill in with imports we have now
set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  "${CMAKE_MODULE_PATH}"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Release")

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /MT")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif ()

option(BUILD_DOC "Build doxygen" OFF)
option(ENABLE_TESTS "Build unittests" OFF)
option(BUILD_STATIC "Statically link all libraries that can be statically linked" OFF)

include_directories(Sources)
include_directories(Tests)

# Check if Doxygen is installed
find_package(Doxygen)
if (BUILD_DOC AND DOXYGEN_FOUND)
    # Set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Request to configure the file
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

# Windows specific options
if (MSVC OR MSYS OR MINGW)
    message("You're running Windows! Make sure your paths are set correctly.")
    set(BUILD_STATIC ON)

    if (MSYS OR MINGW)
        # Add delay loaded library so npcap can be loaded properly
        execute_process(COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/build_scripts/generateDelayLoadedWPCap.bat"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build_scripts
        )
            
        # Add WinSock, liphlpapi, wlanapi and ole32 to the platform specific libraries, MINGW needs it.
        set(CMAKE_CXX_STANDARD_LIBRARIES "-liphlpapi -lwlanapi -lole32 -lwinmm -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
    
        # Needed because otherwise the libraries cannot be found.
        set(Boost_ARCHITECTURE "-x64")
    else()
        string(APPEND PLATFORM_SPECIFIC_LIBRARIES "delayimp.lib")
    endif ()
    
    set(PCAP_ROOT_DIR "c:/npcapsdk")

    set(PDCURSES_INCLUDE_DIR "c:/pdcurses/")
    set(PDCURSES_LIBRARY "c:/pdcurses/wincon/static/pdcurses.a")
    find_package(PDCurses REQUIRED)
    
    set(CURSES_INCLUDE_DIRS "c:/pdcurses/")
    set(CURSES_LIBRARIES ${PDCURSES_LIBRARIES})

    # The logger module uses ERROR in an enum class, GDI also uses this, but screw GDI
    # We have no GUI anyway!
    add_definitions( -DNOGDI )
else ()
    find_package(Curses REQUIRED)
    find_package(LibNL REQUIRED)
endif ()

if (BUILD_STATIC)
    message("Static linking enabled")
    set(BUILD_STATIC ON)
    set(BOOST_ROOT "/home/codedwrench/src/boost")
    add_definitions(-DBUILD_STATIC)
    set(Boost_USE_STATIC_LIBS ON)
endif()

# Concepts seems to break on a bunch of compilers, see: https://github.com/boostorg/asio/issues/312
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

find_package(PCAP REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS chrono date_time atomic system program_options regex)

# TODO: Make this search for source files automatically, this is very ugly!
add_executable(xlinkhandheldassistant main.cpp
        Sources/Handler8023.cpp
        Sources/Handler80211.cpp
        Sources/Logger.cpp
        Sources/PCapReader.cpp
        Sources/WindowModel.cpp
        Sources/MonitorDevice.cpp
        Sources/Parameter80211Reader.cpp
        Sources/XLinkKaiConnection.cpp
        Sources/UserInterface/Button.cpp
        Sources/UserInterface/CheckBox.cpp
        Sources/UserInterface/NetworkingWindow.cpp
        Sources/RadioTapReader.cpp
        Sources/WirelessPSPPluginDevice.cpp
        Sources/UserInterface/String.cpp
        Sources/UserInterface/TextField.cpp
        Sources/UserInterface/UIObject.cpp
        Sources/UserInterface/Window.cpp
        Sources/UserInterface/WindowController.cpp
        Sources/UserInterface/XLinkWindow.cpp
        Includes/Handler8023.h
        Includes/Handler80211.h
        Includes/IConnector.h
        Includes/IHandler.h
        Includes/IPCapDevice.h
        Includes/IWifiInterface.h
        Includes/Logger.h
        Includes/NetworkingHeaders.h
        Includes/Parameter80211Reader.h
        Includes/PCapReader.h
        Includes/RadioTapReader.h
        Includes/MonitorDevice.h
        Includes/XLinkKaiConnection.h
        Includes/WirelessPSPPluginDevice.h
        Includes/UserInterface/Button.h
        Includes/UserInterface/CheckBox.h
        Includes/UserInterface/IUIObject.h
        Includes/UserInterface/IWindow.h
        Includes/UserInterface/NCursesKeys.h
        Includes/NetConversionFunctions.h
        Includes/UserInterface/NetworkingWindow.h
        Includes/UserInterface/String.h
        Includes/UserInterface/TextField.h
        Includes/UserInterface/UIObject.h
        Includes/UserInterface/Window.h
        Includes/UserInterface/WindowController.h
	    Includes/WindowModel.h
        Includes/UserInterface/XLinkWindow.h)
        
if(WIN32)
    if(MSYS)
        set_property(TARGET xlinkhandheldassistant APPEND PROPERTY LINK_FLAGS "/DELAYLOAD:wpcap.dll")
    endif()
    target_sources(xlinkhandheldassistant PRIVATE Sources/WifiInterfaceWindows.cpp Includes/WifiInterfaceWindows.h)
elseif (UNIX)
    target_sources(xlinkhandheldassistant PRIVATE Sources/WifiInterfaceLinuxBSD.cpp Includes/WifiInterfaceLinuxBSD.h)
endif()


if (BUILD_STATIC)    
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ ${CMAKE_EXE_LINKER_FLAGS}")
endif ()


target_include_directories(xlinkhandheldassistant PRIVATE ${PCAP_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${CURSES_INCLUDE_DIRS} ${LibNL_INCLUDE_DIR})
target_link_libraries(xlinkhandheldassistant ${PCAP_LIBRARY} ${Boost_LIBRARIES} ${CURSES_LIBRARIES} ${PLATFORM_SPECIFIC_LIBRARIES} ${LibNL_LIBRARIES})

if (ENABLE_TESTS)
    find_package(GTest REQUIRED)
    include(GoogleTest)
    enable_testing()
    add_executable(tests Tests/PacketHandling_Test.cpp
            Tests/WindowModel_Test.cpp
            Sources/Handler8023.cpp
            Sources/Handler80211.cpp
            Sources/Logger.cpp
            Sources/MonitorDevice.cpp
            Sources/Parameter80211Reader.cpp
            Sources/PCapReader.cpp
            Sources/RadioTapReader.cpp
            Sources/WindowModel.cpp
            Sources/XLinkKaiConnection.cpp)
    target_include_directories(tests PRIVATE ${PCAP_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
    target_link_libraries(tests gtest gmock gtest_main ${PCAP_LIBRARY} ${Boost_LIBRARIES})
    gtest_discover_tests(tests)
endif(ENABLE_TESTS)
