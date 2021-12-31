cmake_minimum_required(VERSION 3.5.1)

set (openauto_VERSION_MAJOR 2)
set (openauto_VERSION_MINOR 1)
set (openauto_VERSION_PATCH 0)

project(openauto 
VERSION ${openauto_VERSION_MAJOR}.${openauto_VERSION_MINOR}.${openauto_VERSION_PATCH}
LANGUAGES CXX)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(base_directory ${CMAKE_CURRENT_SOURCE_DIR})					   
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

include(${base_directory}/cmake_modules/gitversion.cmake)
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
        math(EXPR RPI_MODEL "(0x${RPI_REVISION}>>4)&0xFF")
        message( "-- Raspberry Pi Model: ${RPI_MODEL}" )
        if(RPI_MODEL EQUAL 17)
            message("Raspberry Pi 4 Found")
            add_definitions(-DPI4)
        endif(RPI_MODEL EQUAL 17)
    endif(RPI_BUILD)
message(STATUS "${GST_LIBRARIES}")
endif(GST_BUILD)

add_subdirectory(btservice_proto)
set(BTSERVICE_PROTO_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${BTSERVICE_PROTO_INCLUDE_DIRS})

add_subdirectory(openauto)
add_subdirectory(autoapp)
add_dependencies(autoapp btservice_proto)

set (openauto_VERSION_PATCH ${_build_version})
set (openauto_VERSION_STRING ${openauto_VERSION_MAJOR}.${openauto_VERSION_MINOR}.${openauto_VERSION_PATCH})
set_target_properties(openauto PROPERTIES VERSION ${openauto_VERSION_STRING}
                                          SOVERSION ${openauto_VERSION_MAJOR})
message(INFO " Project Version: ${openauto_VERSION_STRING}")

install(DIRECTORY lib DESTINATION lib COMPONENT libraries)
install(DIRECTORY include DESTINATION include COMPONENT headers)
install(DIRECTORY bin DESTINATION bin COMPONENT applications)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenDsh") #required
SET(CPACK_PACKAGE_VENDOR "OpenDsh") 
set(CPACK_PACKAGE_VERSION ${openauto_VERSION_STRING})
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_COMPONENTS_ALL applications libraries headers Unspecified)
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Applications")
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers")
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION
  "Applications provided by OpenAuto")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION
  "Static libraries used to build programs with OpenAuto")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION
  "C/C++ header files for use with OpenAuto")
set(CPACK_COMPONENT_LIBRARIES_GROUP "Development")
set(CPACK_COMPONENT_HEADERS_GROUP "Development")
set(CPACK_COMPONENT_GROUP_DEVELOPMENT_EXPANDED ON)
set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION
  "All of the tools you'll ever need to develop software")
set(CPACK_COMPONENT_HEADERS_DEPENDS libraries)
set(CPACK_COMPONENT_APPLICATIONS_DEPENDS libraries)
set(CPACK_ALL_INSTALL_TYPES Full Developer)
set(CPACK_INSTALL_TYPE_FULL_DISPLAY_NAME "Everything")
set(CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Developer Full)
set(CPACK_COMPONENT_HEADERS_INSTALL_TYPES Developer Full)
set(CPACK_COMPONENT_APPLICATIONS_INSTALL_TYPES Full)
INCLUDE(CPack)												 