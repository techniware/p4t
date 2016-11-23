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

# Computes file list & directory
set(CMAKE_OUTPUT  ${CMAKE_BINARY_DIR}/output )

if(CMAKE_SYSTEM MATCHES "Linux")
    add_definitions(-DUNIX) 
endif() 

compute_files()

add_executable( ${MODULE_NAME} 
				${SRCS} 
				${INCS}
				${RESOURCES}
)

set_property(TARGET ${MODULE_NAME} PROPERTY FOLDER "Applications")

# Link to dependencies
set (DEP_LIBS "")
foreach (L ${LIB_DEPENDENCIES})
	list (APPEND DEP_LIBS "${L}")
endforeach ()

target_link_libraries( ${MODULE_NAME} ${SYSLIBS} ${DEP_LIBS})

# Add doc generation rules
#add_doc(${MODULE_NAME})

# Reset file lists
set(RESOURCES	"")
set(INCS 		"")
set(SRCS		"")

install( TARGETS	${MODULE_NAME} 
		 DESTINATION	applications
		 COMPONENT 	applications)
