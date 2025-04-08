cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enforce out-of-source build
if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message(FATAL_ERROR "Please build the project from a build folder.\nCreate a separate build directory and run CMake from there:\n  mkdir build\n  cd build\n  cmake <buid_options> ..\n\nYou may delete the CMakeCache.txt file and the CMakeFiles folder that has been created.\n" )
endif()

project(ACS)
set(CODE_VERSION "v1.1.0")

# setting flags for optimization and debug builds
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -ggdb")

# setting the default verbose/warn/intchecks (edited with the -D flag)
set(VERBOSE "0" CACHE STRING "Verbose setting set from command line")
set(LOG "0" CACHE STRING "Flag to enable log on file")
#set(WARN "0" CACHE STRING "Flag to enable warning messages")
#set(INTCHECK "0" CACHE STRING "Flag to enable integrity checks")
# Detect machine and set CPLEX paths accordingly
if(EXISTS "/Applications/CPLEX_Studio2211/cplex/lib/arm64_osx/static_pic")                  # Mac Laptop
    set(CPLEX_DIR "/Applications/CPLEX_Studio2211/cplex/lib/arm64_osx/static_pic")
    set(CPLEX_INCLUDE "/Applications/CPLEX_Studio2211/cplex/include/ilcplex")
elseif(EXISTS "/opt/ibm/ILOG/CPLEX_Studio2212/cplex/lib/x86-64_linux/static_pic")           # Linux PC
    set(CPLEX_DIR "/opt/ibm/ILOG/CPLEX_Studio2212/cplex/lib/x86-64_linux/static_pic")
    set(CPLEX_INCLUDE "/opt/ibm/ILOG/CPLEX_Studio2212/cplex/include/ilcplex")
elseif(EXISTS "/nfsd/rop/sw/ibm/cos221/cplex/lib/x86-64_linux/static_pic")                  # Cluster DEI
    set(CPLEX_DIR "/nfsd/rop/sw/ibm/cos221/cplex/lib/x86-64_linux/static_pic")
    set(CPLEX_INCLUDE "/nfsd/rop/sw/ibm/cos221/cplex/include/ilcplex")
else()
    message(FATAL_ERROR "CPLEX directory not found on this machine. Please set CPLEX_DIR manually.")
endif()

# check if all necessary folders exist
cmake_path(GET CMAKE_HOME_DIRECTORY PARENT_PATH HOME_DIR)
set(LOGS_DIR "${HOME_DIR}/code/log")
set(CPXOUT_DIR "${HOME_DIR}/code/log/cplex_out")
set(CPXLOG_DIR "${CPXOUT_DIR}/log")
set(CPXLP_DIR "${CPXOUT_DIR}/mip")
set(DATA_DIR "${HOME_DIR}/code/data")
if(NOT EXISTS ${LOGS_DIR})
    file(MAKE_DIRECTORY ${LOGS_DIR})
    message(STATUS "Directory ${LOGS_DIR} created.")
endif()
if(NOT EXISTS ${CPXOUT_DIR})
    file(MAKE_DIRECTORY ${CPXOUT_DIR})
    message(STATUS "Directory ${CPXOUT_DIR} created.")
endif()
if(NOT EXISTS ${CPXLP_DIR})
    file(MAKE_DIRECTORY ${CPXLP_DIR})
    message(STATUS "Directory ${CPXLP_DIR} created.")
endif()
if(NOT EXISTS ${CPXLOG_DIR})
    file(MAKE_DIRECTORY ${CPXLOG_DIR})
    message(STATUS "Directory ${CPXLOG_DIR} created.")
endif()
if(NOT EXISTS ${CPLEX_DIR})
    message(FATAL_ERROR "Directory ${CPLEX_DIR} doesn't exist... Please set with -DCPLEX_DIR=<path> the correct path to CPLEXs static pic folder.")
endif()
if(NOT EXISTS ${CPLEX_INCLUDE})
    message(FATAL_ERROR "Directory ${CPLEX_INCLUDE} doesn't exist... Please set with -DCPLEX_INCLUDE=<path> the correct path to CPLEXs headers folder.")
endif()
if(NOT EXISTS ${DATA_DIR})
    message(FATAL_ERROR "Directory ${INST_DIR} doesn't exist... Please create it and put the instances in there")
endif()

# build

include_directories(include)
include_directories(${CPLEX_INCLUDE})

add_library(
    Utils
    source/Utils.cpp
    include/Utils.hpp
)

add_library(
    MIP
    source/MIP.cpp
    include/MIP.hpp
)

add_library(
    FMIP
    source/FMIP.cpp
    include/FMIP.hpp
)
add_library(
    RlxFMIP
    source/RlxFMIP.cpp
    include/RlxFMIP.hpp
)

add_library(
    OMIP
    source/OMIP.cpp
    include/OMIP.hpp
)

add_library(
    MTContext
    source/MTContext.cpp
    include/MTContext.hpp
)

add_library(
    FixPol
    source/FixPolicy.cpp
    include/FixPolicy.hpp
)

add_library(
    MerPol
    source/MergePolicy.cpp
    include/MergePolicy.hpp
)


# ...

find_library(CPLEX_LIBRARY
    NAMES cplex
    PATHS ${CPLEX_DIR}
    NO_DEFAULT_PATH
)

if(NOT CPLEX_LIBRARY)
    message(FATAL_ERROR "CPLEX library not found in ${CPLEX_DIR}")
endif()

# Add the main executable
add_executable(
    ACS
    source/ACS.cpp
)

add_executable(
    CPLEXRun
    source/CPLEXRun.cpp
)

# Linking
target_link_libraries(
    ACS
    MIP
    FMIP
    OMIP
    MTContext
    FixPol
    RlxFMIP
    MerPol
    Utils
    #pthread
    pthread
    dl
    # Link CPLEX library
    ${CPLEX_LIBRARY}
)

target_link_libraries(
    CPLEXRun
    MIP
    Utils
    #pthread
    pthread
    dl
    # Link CPLEX library
    ${CPLEX_LIBRARY}
)

# adding definitions
add_definitions(-DACS_HOME_DIR="${HOME_DIR}"
                -DACS_VERBOSE=${VERBOSE} 
                -DACS_WARN=${WARN} 
                -DACS_INTCHECK=${INTCHECK} 
                -DLOG=${LOG} 
                -DCODE_VERSION=${CODE_VERSION}) #ACS for us

# custom builds
add_custom_target(opt
    COMMAND echo "Code version: ${CODE_VERSION}"
    COMMAND echo "Flags used: ${CMAKE_CXX_FLAGS_RELEASE}"
    COMMAND ${CMAKE_COMMAND} -D CMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ACS
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target CPLEXRun
)

add_custom_target(debug
    COMMAND echo "Code version: ${CODE_VERSION}"
    COMMAND echo "Flags used: ${CMAKE_CXX_FLAGS_DEBUG} -VERBOSE=${VERBOSE} -LOG=${LOG}"
    COMMAND ${CMAKE_COMMAND} -D CMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ACS
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target CPLEXRun
)

add_custom_target(clear
    COMMAND rm -r ${CMAKE_HOME_DIRECTORY}/build/*
)

message(STATUS "Code version: ${CODE_VERSION}")