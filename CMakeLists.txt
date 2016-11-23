cmake_minimum_required(VERSION 2.8.4)
project(P4T)


# Global settings
set(GLOBAL_OUTPUT_PATH ${PROJECT_BINARY_DIR}/install)

# Sets global output directory for single configuration (GCC)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GLOBAL_OUTPUT_PATH})

# Sets global output directory for sub-configurations (msvc, mingw)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GLOBAL_OUTPUT_PATH})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GLOBAL_OUTPUT_PATH})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GLOBAL_OUTPUT_PATH})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)


# 3rdparty Libraries
include(3rdparty/CMakeLists-ExternalProjects.txt)


# Global compiler flags
if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4X]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++14")
else()

endif()

# Global link directories
link_directories(${GLOBAL_OUTPUT_PATH}/lib)

# Global include directories
include_directories(${GLOBAL_OUTPUT_PATH}/include)

add_subdirectory(Variables)

# Installation
# Copies files from build directory into install directory.
#file(GLOB executables "${GLOBAL_OUTPUT_PATH}/*.exe")
#install(FILES ${executables} DESTINATION "bin")

#file(GLOB libraries "${GLOBAL_OUTPUT_PATH}/*.dll")
#install(FILES ${libraries} DESTINATION "bin")

 
# SDK Installation
#install(DIRECTORY "src/lib1/include/" DESTINATION "include" FILES_MATCHING PATTERN "*.h")
#install(DIRECTORY "src/lib2/include/" DESTINATION "include" FILES_MATCHING PATTERN "*.h")


# DEV STUFF
#
#install(TARGETS lib1 lib2 app1 app2
#  RUNTIME DESTINATION ${INSTALL_PATH}/bin
#  LIBRARY DESTINATION ${INSTALL_PATH}/bin
#  ARCHIVE DESTINATION ${INSTALL_PATH}/bin
#)