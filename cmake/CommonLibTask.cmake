#                _      _   _ ______  _____ 
#               | |    | \ | || ___ \|____ |
#               | |    |  \| || |_/ /    / /
#               | |    | . ` ||    /     \ \
#               | |____| |\  || |\ \ .___/ /
#               \_____/\_| \_/\_| \_|\____/ 
#                                           
#
#            Copyright (c) 2016, Nexeya System.
#                   All rights reserved.
#
# This is unpublished proprietary source code of Nexeya System.
# The contents of this file may not be disclosed to third parties, 
# copied or duplicated in any form, in whole or in part.
#
# @author: Laurent Carcagno - Techniware
#
# @date:   05/10/2016
#
#                                    _                     
#       _/7           (((           ((_           ...      
#      (o o)         (o o)         (o o)         (o -)     
#  ooO--(_)--Ooo-ooO--(_)--Ooo-ooO--(_)--Ooo-ooO--(_)--Ooo-   
#
#
include( ${CMAKE_SOURCE_DIR}/cmake/Utils.cmake)

message(STATUS "Adding module ${MODULE_NAME}")

# Computes file list & directory
set(CMAKE_OUTPUT  ${CMAKE_BINARY_DIR}/output )

if(CMAKE_SYSTEM MATCHES "Linux")
    add_definitions(-DUNIX) 
endif() 

computes_outputdir()
compute_files()

add_library( ${MODULE_NAME} 
             ${LIB_MODE}
             ${SRCS}
             ${PRIVATES}
             ${INCS} 
             ${PRIVATE_INCS}
             ${RESOURCES})

# Link to dependencies
set (DEP_LIBS "")
foreach (L ${LIB_DEPENDENCIES})
	list (APPEND DEP_LIBS "${L}")
endforeach ()


target_link_libraries( ${MODULE_NAME} ${SYSLIBS} ${DEP_LIBS})

if (CMAKE_SYSTEM_NAME  MATCHES "Linux")
    target_link_libraries( ${MODULE_NAME} rt dl)
endif()

# Add compilation flags specific to library
add_definitions(-D_DLL -D_USRDLL)

# Add doc generation rules
#add_doc(${MODULE_NAME})

set_property(TARGET ${MODULE_NAME} PROPERTY FOLDER "Runtime")

install( TARGETS	${MODULE_NAME} 
	 DESTINATION	lib/nexeya_gpio
	 COMPONENT 	libraries)


# Reset file lists
set(RESOURCES	 "")
set(INCS 		 "")
set(SRCS		 "")
