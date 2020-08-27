cmake_minimum_required(VERSION 3.5.1)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

project(openauto CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake_modules/")
SET(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -Wall -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
add_definitions(-DBOOST_ALL_DYN_LINK)
find_package(Boost REQUIRED COMPONENTS system log OPTIONAL_COMPONENTS unit_test_framework)

find_package(libusb-1.0 REQUIRED)
find_package(Qt5 COMPONENTS Multimedia MultimediaWidgets Bluetooth Qml Quick QuickWidgets REQUIRED)
find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(rtaudio REQUIRED)
find_package(aasdk REQUIRED)
find_package(Threads)

if(RPI_BUILD)
    find_package(libomx)
endif()

if(GST_BUILD)
    find_package(GObject)
    find_package(Qt5GStreamer)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GST REQUIRED
                      gstreamer-1.0>=1.4
                      gstreamer-sdp-1.0>=1.4
                      gstreamer-video-1.0>=1.4
                      gstreamer-app-1.0>=1.4)
    add_definitions(-DUSE_GST)
    if(RPI_BUILD)
        add_definitions(-DRPI)
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/functions.cmake)
        findRpiRevision( RPI_REVISION )
        set(RPI_4_REVISIONS a03111 b03111 b03112 c03111 c03112 d03114)
        if(RPI_REVISION IN_LIST RPI_4_REVISIONS)
            message("Raspberry Pi 4 Found")
            add_definitions(-DPI4)
        endif(RPI_REVISION IN_LIST RPI_4_REVISIONS)
    endif(RPI_BUILD)
message(STATUS "${GST_LIBRARIES}")
endif(GST_BUILD)

add_subdirectory(btservice_proto)
set(BTSERVICE_PROTO_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${BTSERVICE_PROTO_INCLUDE_DIRS})

add_subdirectory(openauto)
add_subdirectory(autoapp)
add_dependencies(autoapp btservice_proto)
