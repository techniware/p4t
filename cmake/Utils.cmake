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
if (__UTILS_INCLUDED)
  return ()
else ()
  set (__COMMONTOOLS_INCLUDED TRUE)
endif ()
set (NAMESPACE_SEPARATOR "@")
 
 ##############################################################################
 # @brief Get namespace of build target.
 #
 # @param [out] TARGET_NS  Namespace part of target UID. If @p TARGET_UID is
 #                         no UID, i.e., does not contain a namespace part,
 #                         the namespace of this project is returned.
 # @param [in]  TARGET_UID Target UID/name.
 function (target_namespace TARGET_NS TARGET_UID)
   if (${TARGET_UID} MATCHES "${NAMESPACE_SEPARATOR}")
     string (REGEX REPLACE "${NAMESPACE_SEPARATOR}.*$" "" TMP "${TARGET_UID}")
     set ("${TARGET_NS}" "${TMP}" PARENT_SCOPE)
   else ()
     set ("${TARGET_NS}" "${NAMESPACE}" PARENT_SCOPE)
   endif ()
 endfunction ()
 
 
 ##############################################################################
 # @brief Get "global" target name, i.e., actual CMake target name.
 #
 # In order to ensure that CMake target names are unique across BASIS projects,
 # the target name used by a developer of a BASIS project is converted by this
 # function into another target name which is used as acutal CMake target name.
 #
 # The function target_name() can be used to convert the unique target
 # name, the target UID, back to the original target name passed to this
 # function.
 #
 # @sa target_name()
 # @sa USE_TARGET_UIDS
 #
 # @param [out] TARGET_UID  "Global" target name, i.e., actual CMake target name.
 # @param [in]  TARGET_NAME Target name used as argument to BASIS CMake functions.
 #
 # @returns Sets @p TARGET_UID to the UID of the build target @p TARGET_NAME.
 
 function (target_uid TARGET_UID TARGET_NAME)
   if (USE_TARGET_UIDS)
     if (TARGET "${TARGET_NAME}" OR TARGET_NAME MATCHES "${NAMESPACE_SEPARATOR}")
       set ("${TARGET_UID}" "${TARGET_NAME}" PARENT_SCOPE)
     else ()
       set ("${TARGET_UID}" "${NAMESPACE}${NAMESPACE_SEPARATOR}${TARGET_NAME}" T_SCOPE)
     endif ()
   else ()
     target_namespace (TARGET_NS "${TARGET_NAME}")
     if ("${TARGET_NS}" STREQUAL "${NAMESPACE}")
       target_name (TARGET_NAME "${TARGET_NAME}")
     endif ()
     set ("${TARGET_UID}" "${TARGET_NAME}" PARENT_SCOPE)
   endif ()
 endfunction ()
 
 ##############################################################################
 # @brief Get "local" target name, i.e., BASIS target name.
 #
 # @sa target_uid()
 # @sa USE_TARGET_UIDS
 #
 # @param [out] TARGET_NAME Target name used as argument to BASIS functions.
 # @param [in]  TARGET_UID  "Global" target name, i.e., actual CMake target name.
 #
 # @returns Sets @p TARGET_NAME to the name of the build target with UID @p TARGET_UID.
 
 function (target_name TARGET_NAME TARGET_UID)
   string (REGEX REPLACE "^.*${NAMESPACE_SEPARATOR}" "" TMP "${TARGET_UID}")
   set ("${TARGET_NAME}" "${TMP}" PARENT_SCOPE)
 endfunction ()
 
 ##############################################################################
 # @brief Checks whether a given name is a valid target name.
 #
 # Displays fatal error message when target name is invalid.
 #
 # @param [in] TARGET_NAME Desired target name.
 #
 # @returns Nothing.
 #
 # @ingroup CMakeUtilities
 
 function (check_target_name TARGET_NAME)
   # reserved target name ?
   list (FIND RESERVED_TARGET_NAMES "${TARGET_NAME}" IDX)
   if (NOT IDX EQUAL -1)
     message (FATAL_ERROR "Target name \"${TARGET_NAME}\" is reserved and cannot be used.")
   endif ()
 
   # invalid target name ?
   if (TARGET_NAME MATCHES " ")
     message (FATAL_ERROR "Target name ${TARGET_NAME} is invalid. Target names cannot contain whitespaces.")
   endif ()
 
   if (TARGET_NAME MATCHES "${NAMESPACE_SEPARATOR}")
     message (FATAL_ERROR "Target name ${TARGET_NAME} is invalid. Target names cannot"
                          " contain string '${NAMESPACE_SEPARATOR}'.")
   endif ()
 
   # unique ?
   target_uid (TARGET_UID "${TARGET_NAME}")
 
   if (TARGET "${TARGET_UID}")
     message (FATAL_ERROR "There exists already a target named ${TARGET_UID}."
                          " Target names must be unique.")
   endif ()
 endfunction ()
 ## @}

 # Doxygen - API documentation
 find_package (Doxygen)

 # ============================================================================
 # helper
 # ============================================================================
 
 # adding / generating documentation
 function (add_doc TARGET_NAME)
    check_target_name ("${TARGET_NAME}_doc")
    target_uid (TARGET_UID "${TARGET_NAME}")
    message (STATUS "Adding target ${TARGET_UID}_doc.")
 
    # lower target name is used, for example, for default DESTINATION
    string (TOLOWER "${TARGET_NAME}" TARGET_NAME_LOWER)

    message (STATUS "Adding documentation target for ${TARGET_UID}...")
 
    # Doxygen found ?
    if (BUILD_DOCUMENTATION)
        set (ERRMSGTYP "")
        set (ERRMSG    "failed")
    else ()
        set (ERRMSGTYP "STATUS")
        set (ERRMSG    "skipped")
    endif ()
 
    if (NOT DOXYGEN_EXECUTABLE)
       message (${ERRMSGTYP} "Doxygen not found. Skipping build of ${TARGET_UID}_doc.")
       return ()
    endif ()
     
    if (NOT EXISTS  ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
        message (${ERRMSGTYP}  "Missing Doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile does not exist.")
    else ()      
        # add target
        set (CMAKE_DOC_DIR ${CMAKE_BINARY_DIR}/output/doc/${TARGET_NAME})
        string(REPLACE "/" "\\" DOCDIR ${CMAKE_DOC_DIR})

        file(MAKE_DIRECTORY ${CMAKE_DOC_DIR}) 
        add_custom_target (
            ${TARGET_UID}_doc
            COMMAND  SET PROJECT_ROOT_DIR=${CMAKE_SOURCE_DIR}
            COMMAND  SET PROJECT_BUILD_DIR=${CMAKE_DOC_DIR}
            COMMAND  SET COMPOUND_NAME=${TARGET_NAME}
            COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
            COMMAND MOVE  "${DOCDIR}\\html\\${TARGET_NAME}.chm" "${DOCDIR}\\${TARGET_NAME}.chm"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
         
        # add target as dependency to doc target
        #if (NOT TARGET doc)
        #   add_custom_target(doc)
        #endif ()
        #add_dependencies (doc ${TARGET_UID}_doc)
        
        set_property(TARGET ${TARGET_UID}_doc PROPERTY FOLDER documentations )
    
         message (STATUS "Adding documentation ${TARGET_UID} done.")
     endif()
 endfunction ()

 
macro ( compute_files )
    
    #
    # Computes local source files list
    #    
    if ( "${SRCS}" STREQUAL "") 
        aux_source_directory( ${MODULE_ROOTDIR}/src SRCS )
    endif()

    if ( "${PRIVATES}" STREQUAL "") 
        aux_source_directory( ${MODULE_ROOTDIR}/private/src PRIVATES )
    endif()
        
    #
    # Computes local include files list
    #
    
    if ( "${INCS}" STREQUAL "") 
        file(    GLOB_RECURSE 
                FULL_PATH_INCS 
                "${MODULE_ROOTDIR}/include/*.h" 
                "${MODULE_ROOTDIR}/include/*.hh" 
                "${MODULE_ROOTDIR}/include/*.inl" )

        foreach (SOURCE ${FULL_PATH_INCS})
            file(RELATIVE_PATH INC ${CMAKE_CURRENT_SOURCE_DIR}/ ${SOURCE})
            list (APPEND INCS "${INC}")
        endforeach ()
    endif()

    
    if ( "${PRIVATE_INCS}" STREQUAL "") 
        file(    GLOB_RECURSE 
                FULL_PATH_INCS 
                "${MODULE_ROOTDIR}/private/include/*.h" 
                "${MODULE_ROOTDIR}/private/include/*.hh" 
                "${MODULE_ROOTDIR}/private/include/*.inl" )

        foreach (SOURCE ${FULL_PATH_INCS})
            file(RELATIVE_PATH INC ${CMAKE_CURRENT_SOURCE_DIR}/ ${SOURCE})
            list (APPEND PRIVATE_INCS "${INC}")
        endforeach ()
    endif()    

    #
    # Computes local resource files list
    #
    if ( "${RESOURCES}" STREQUAL "") 
        file(   GLOB 
                FULL_PATH_RES 
                "${MODULE_ROOTDIR}/*.properties"  
                "${MODULE_ROOTDIR}/resources/*"  
                "${MODULE_ROOTDIR}/*.xml" 
                "${MODULE_ROOTDIR}/Doxyfile" 
                "${MODULE_ROOTDIR}/bundle/*" "*.bndlspec")
        foreach (RESOURCE ${FULL_PATH_RES})
            file(RELATIVE_PATH INC ${CMAKE_CURRENT_SOURCE_DIR}/ ${RESOURCE})
            list (APPEND RESOURCES "${RESOURCE}" )
        endforeach ()
    endif()

    # Group file by kind for easy browsing
    source_group(    "Headers files" 
                    FILES ${INCS})

                                        
    source_group(    "Sources files"
                    FILES ${SRCS})
    
    source_group(    "Resource files" 
                    FILES ${RESOURCES})

endmacro()
