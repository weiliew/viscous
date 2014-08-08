 #  Created on: 1 Jan 2013
 #      Author: Wei Liew [wei@onesixeightsolutions.com]
 #
 #  Copyright Wei Liew 2012 - 2014.
 #  Distributed under the Boost Software License, Version 1.0.
 #  (See http://www.boost.org/LICENSE_1_0.txt)
 
# Project Name
project (ViscousFraamework CXX C)

#INCLUDE(CTest)
set (ENABLE_TESTS true)

set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_CLASS Beta)

INCLUDE (${PROJECT_SOURCE_DIR}/CMakeMacros.txt)
INCLUDE (${PROJECT_SOURCE_DIR}/thirdparty/CMakeLists.txt)

# Get GIT version

# get compiler version
execute_process (
    COMMAND ${CMAKE_CXX_COMPILER} --version
    COMMAND head -n 1
    OUTPUT_VARIABLE COMPILER_VERSION)
string (REGEX REPLACE "\n" "" COMPILER_VERSION ${COMPILER_VERSION})

# Minimum required cmake version
cmake_minimum_required(VERSION 2.8)

#set (CMAKE_BUILD_TYPE Release)

# Build debug by default. 
# to specify on command line, use cmake -D CMAKE_BUILD_TYPE=Debug|Release
if (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpic -D_REENTRANT -std=c++11 -Woverloaded-virtual")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic -D_REENTRANT")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fpic -D_REENTRANT -std=c++11 -Woverloaded-virtual -ggdb3 -O0 -W -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
set (CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -fpic -D_REENTRANT -ggdb3 -O0 -W -fprofile-arcs -ftest-coverage")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build/bin)
set(LIBRARY_OUTPUT_PATH    ${PROJECT_BINARY_DIR}/build/lib)

if(NOT EXISTS ${EXECUTABLE_OUTPUT_PATH})
    FILE(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
endif(NOT EXISTS ${EXECUTABLE_OUTPUT_PATH})

if(NOT EXISTS ${LIBRARY_OUTPUT_PATH})
    FILE(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
endif(NOT EXISTS ${LIBRARY_OUTPUT_PATH})


set (MY_COMMON_LIBS    libpthread.so libm.so librt.so libdl.so)
set (MY_BOOST_LIBS     libboost_filesystem.a libboost_date_time.a libboost_thread.a libboost_system.a libboost_program_options.a)

include_directories (
    ${MY_BOOST_DIR}/include
    ${PROJECT_SOURCE_DIR}/framework
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_BINARY_DIR}/build/include
)


link_directories(
    ${MY_BOOST_DIR}/lib/
    ${LIBRARY_OUTPUT_PATH}
    ${MY_INSTALL_DIR}/lib
    )

# Add linker flag
add_definitions (-D__STDC_FORMAT_MACROS)

include_directories (framework)
add_subdirectory (framework)

# Apps build
add_subdirectory (apps)


