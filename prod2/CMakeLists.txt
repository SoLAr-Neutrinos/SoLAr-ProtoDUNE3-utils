######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : CMakeLists.txt
# @created     : Tuesday Nov 18, 2025 16:20:20 CET
######################################################################

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(visibility_map LANGUAGES CXX C)

list(APPEND CMAKE_PREFIX_PATH 
  $ENV{ROOTSYS} 
  CMAKE_LIBRARY_OUTPUT_DIRECTORY)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_CONFIGURATION_TYPES Debug Release)

if (CMAKE_BUILD_TYPE STREQUAL "Debug") 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  message(STATUS "DEBUG: Adding '-g' option for gdb debugging")
endif()


find_package(ROOT REQUIRED COMPONENTS RIO Core)
include(${ROOT_USE_FILE})

find_package(SOLAr-sim CONFIG REQUIRED)
if(SOLAr-sim_FOUND)
  message(STATUS "SOLAr-sim found: libraries at ${SOLARSIM_LIB_DIR}")
  list(APPEND CMAKE_PREFIX_PATH ${SOLARSIM_BIN_DIR})
  list(APPEND CMAKE_PREFIX_PATH ${SOLARSIM_EXT_DIR})
endif()

find_package(RapidJSON REQUIRED)
if (RapidJSON_FOUND)
  message(STATUS "RapidJSON found: include dir at ${RAPIDJSON_INCLUDE_DIRS}")
endif()


include_directories(
  ${RapidJSON_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIRS}
)

add_executable(make_vis_tree make_vis_tree.cc)
add_executable(make_vis_map make_vis_map.cc)

# Executables list
SET(solarpd3_executables
  make_vis_tree
  make_vis_map
)

FOREACH(exe ${solarpd3_executables})
  target_link_libraries( ${exe}
    PRIVATE ROOT::RIO ROOT::Tree ROOT::Core
    SOLARSIM::SLArMCEventReadout
    SOLARSIM::SLArGenRecords
  )

  target_include_directories( ${exe}
    PRIVATE
    ${SOLARSIM_INCLUDE_DIR}
    ${ROOT_INCLUDE_DIRS}
  )

  install(TARGETS ${exe}
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
  )  
ENDFOREACH(exe)


