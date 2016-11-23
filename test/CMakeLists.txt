#                _      _   _ ______  _____ 
#               | |    | \ | || ___ \|____ |
#               | |    |  \| || |_/ /    / /
#               | |    | . ` ||    /     \ \
#               | |____| |\  || |\ \ .___/ /
#               \_____/\_| \_/\_| \_|\____/ 
#                                           
#
#            Copyright (c) 2016, Techniware.
#                   All rights reserved.
#
# This is unpublished proprietary source code of Nexeya System.
# The contents of this file may not be disclosed to third parties, 
# copied or duplicated in any form, in whole or in part.
#
# @author: Laurent Carcagno - Techniware
#
# @date:   12/11/2016
#
#                                    _                     
#       _/7           (((           ((_           ...      
#      (o o)         (o o)         (o o)         (o -)     
#  ooO--(_)--Ooo-ooO--(_)--Ooo-ooO--(_)--Ooo-ooO--(_)--Ooo-   
#
#

#
# MODULE NAME
#
set(MODULE_NAME "Test${LIB_EXT}")
set(MODULE_ROOTDIR "${CMAKE_CURRENT_SOURCE_DIR}")

#
# Source files
#
set (   SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp
)

#
# Include directories
#
include_directories( include )

# System specfic libraries
if (NOT CMAKE_SYSTEM MATCHES "Linux")
    set(SYSLIBS ws2_32 wsock32) 
endif()

# Module dependencies
#set(	LIB_DEPENDENCIES 
#)

#add_definitions(	
#)

#
# Add the module to the build system
#
include( ${CMAKE_SOURCE_DIR}/cmake/CommonExeTask.cmake )

