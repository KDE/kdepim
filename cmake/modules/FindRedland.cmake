# - Try to find Redland
# Once done this will define
#
#  REDLAND_FOUND       - system has Redland
#  REDLAND_LIBRARIES   - Link these to use REDLAND
#  REDLAND_DEFINITIONS - Compiler switches required for using REDLAND
#  REDLAND_VERSION     - The redland version string

# (c) 2007 Sebastian Trueg <trueg@kde.org>
#
# Based on FindFontconfig Copyright (c) 2006,2007 Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


INCLUDE(MacroEnsureVersion)


#if (REDLAND_LIBRARIES AND REDLAND_INCLUDE_DIR AND RASQAL_INCLUDE_DIR AND RASQAL_LIBRARIES AND RAPTOR_LIBRARIES)

  # in cache already
#  set(REDLAND_FOUND TRUE)

#else (REDLAND_LIBRARIES AND REDLAND_INCLUDE_DIR and RASQUAL_INCLUDE_DIR AND RASQAL_LIBRARIES AND RAPTOR_LIBRARIES)

  FIND_PROGRAM(
    REDLAND_CONFIG
    NAMES redland-config
    )

  if(REDLAND_CONFIG)
    EXECUTE_PROCESS(
      COMMAND ${REDLAND_CONFIG} --version
      OUTPUT_VARIABLE REDLAND_VERSION
      )
    if(REDLAND_VERSION)
      STRING(REPLACE "\n" "" REDLAND_VERSION ${REDLAND_VERSION})
  
      # extract include paths from redland-config
      EXECUTE_PROCESS(
        COMMAND ${REDLAND_CONFIG} --cflags
        OUTPUT_VARIABLE redland_LIBS_ARGS)
      STRING( REPLACE " " ";" redland_LIBS_ARGS ${redland_LIBS_ARGS} )
      FOREACH( _ARG ${redland_LIBS_ARGS} )
        IF(${_ARG} MATCHES "^-I")
          STRING(REGEX REPLACE "^-I" "" _ARG ${_ARG})
          STRING( REPLACE "\n" "" _ARG ${_ARG} )
          LIST(APPEND redland_INCLUDE_DIRS ${_ARG})
        ENDIF(${_ARG} MATCHES "^-I")
      ENDFOREACH(_ARG)
  
      # extract lib paths from redland-config
      EXECUTE_PROCESS(
        COMMAND ${REDLAND_CONFIG} --libs
        OUTPUT_VARIABLE redland_CFLAGS_ARGS)
      STRING( REPLACE " " ";" redland_CFLAGS_ARGS ${redland_CFLAGS_ARGS} )
      FOREACH( _ARG ${redland_CFLAGS_ARGS} )
        IF(${_ARG} MATCHES "^-L")
          STRING(REGEX REPLACE "^-L" "" _ARG ${_ARG})
          LIST(APPEND redland_LIBRARY_DIRS ${_ARG})
        ENDIF(${_ARG} MATCHES "^-L")
      ENDFOREACH(_ARG)
    endif(REDLAND_VERSION)
  endif(REDLAND_CONFIG)

  # raptor is not redland, look for it separately
  FIND_PROGRAM(
    RAPTOR_CONFIG
    NAMES raptor-config
    )
  if(RAPTOR_CONFIG)
    EXECUTE_PROCESS(
      COMMAND ${RAPTOR_CONFIG} --version
      OUTPUT_VARIABLE RAPTOR_VERSION
      )
    if(RAPTOR_VERSION)
      STRING(REPLACE "\n" "" RAPTOR_VERSION ${RAPTOR_VERSION})
  
      MACRO_ENSURE_VERSION("1.4.16" ${RAPTOR_VERSION} RAPTOR_HAVE_TRIG)
  
      # extract include paths from raptor-config
      EXECUTE_PROCESS(
        COMMAND ${RAPTOR_CONFIG} --cflags
        OUTPUT_VARIABLE raptor_CFLAGS_ARGS)
      STRING( REPLACE " " ";" raptor_CFLAGS_ARGS ${raptor_CFLAGS_ARGS} )
      FOREACH( _ARG ${raptor_CFLAGS_ARGS} )
        IF(${_ARG} MATCHES "^-I")
          STRING(REGEX REPLACE "^-I" "" _ARG ${_ARG})
          STRING( REPLACE "\n" "" _ARG ${_ARG} )
          LIST(APPEND raptor_INCLUDE_DIRS ${_ARG})
        ENDIF(${_ARG} MATCHES "^-I")
      ENDFOREACH(_ARG)
  
      # extract lib paths from raptor-config
      EXECUTE_PROCESS(
        COMMAND ${RAPTOR_CONFIG} --libs
        OUTPUT_VARIABLE raptor_CFLAGS_ARGS)
      STRING( REPLACE " " ";" raptor_CFLAGS_ARGS ${raptor_CFLAGS_ARGS} )
      FOREACH( _ARG ${raptor_CFLAGS_ARGS} )
        IF(${_ARG} MATCHES "^-L")
          STRING(REGEX REPLACE "^-L" "" _ARG ${_ARG})
          LIST(APPEND raptor_LIBRARY_DIRS ${_ARG})
        ENDIF(${_ARG} MATCHES "^-L")
      ENDFOREACH(_ARG)
    endif(RAPTOR_VERSION)
  else(RAPTOR_CONFIG)
    SET(RAPTOR_VERSION "1.0.0")
  endif(RAPTOR_CONFIG)


  # rasqal is not redland, look for it separately
  FIND_PROGRAM(
    RASQAL_CONFIG
    NAMES rasqal-config
    )

  if(RASQAL_CONFIG)
    EXECUTE_PROCESS(
      COMMAND ${RASQAL_CONFIG} --version
      OUTPUT_VARIABLE RASQAL_VERSION
      )
    if(RASQAL_VERSION)
      STRING(REPLACE "\n" "" RASQAL_VERSION ${RASQAL_VERSION})
  
      # extract include paths from rasqal-config
      EXECUTE_PROCESS(
        COMMAND ${RASQAL_CONFIG} --cflags
        OUTPUT_VARIABLE rasqal_CFLAGS_ARGS)
      STRING( REPLACE " " ";" rasqal_CFLAGS_ARGS ${rasqal_CFLAGS_ARGS} )
      FOREACH( _ARG ${rasqal_CFLAGS_ARGS} )
        IF(${_ARG} MATCHES "^-I")
          STRING(REGEX REPLACE "^-I" "" _ARG ${_ARG})
          STRING( REPLACE "\n" "" _ARG ${_ARG} )
          LIST(APPEND rasqal_INCLUDE_DIRS ${_ARG})
        ENDIF(${_ARG} MATCHES "^-I")
      ENDFOREACH(_ARG)
  
      # extract lib paths from rasqal-config
      EXECUTE_PROCESS(
        COMMAND ${RASQAL_CONFIG} --libs
        OUTPUT_VARIABLE rasqal_CFLAGS_ARGS)
      STRING( REPLACE " " ";" rasqal_CFLAGS_ARGS ${rasqal_CFLAGS_ARGS} )
      FOREACH( _ARG ${rasqal_CFLAGS_ARGS} )
        IF(${_ARG} MATCHES "^-L")
          STRING(REGEX REPLACE "^-L" "" _ARG ${_ARG})
          LIST(APPEND rasqal_LIBRARY_DIRS ${_ARG})
        ENDIF(${_ARG} MATCHES "^-L")
      ENDFOREACH(_ARG)
    endif(RASQAL_VERSION)
  endif(RASQAL_CONFIG)

  # now a hack for win32 (only?)
  # soprano only includes <redland.h> instead <redland/redland.h>
  if(WIN32)
    find_path(REDLAND_INCLUDE_DIR_TMP redland/redland.h
      PATHS
      ${_REDLANDIncDir}
    )
    if(REDLAND_INCLUDE_DIR_TMP)
      set(REDLAND_INCLUDE_DIR ${REDLAND_INCLUDE_DIR_TMP}/redland CACHE PATH "Path to a file.")
    endif(REDLAND_INCLUDE_DIR_TMP)

    find_path(RASQAL_INCLUDE_DIR_TMP redland/rasqal.h
      PATHS
      ${redland_INCLUDE_DIRS}
    )
    if(RASQAL_INCLUDE_DIR_TMP)
      set(RASQAL_INCLUDE_DIR ${RASQAL_INCLUDE_DIR_TMP}/redland CACHE PATH "Path to a file.")
    endif(RASQAL_INCLUDE_DIR_TMP)

    find_path(RAPTOR_INCLUDE_DIR_TMP redland/raptor.h
      PATHS
      ${redland_INCLUDE_DIRS}
    )
    if(RAPTOR_INCLUDE_DIR_TMP)
      set(RAPTOR_INCLUDE_DIR ${RAPTOR_INCLUDE_DIR_TMP}/redland CACHE PATH "Path to a file.")
    endif(RAPTOR_INCLUDE_DIR_TMP)
  else(WIN32)
    find_path(REDLAND_INCLUDE_DIR redland.h
      PATHS
      ${redland_INCLUDE_DIRS}
      /usr/X11/include
    )
    find_path(RASQAL_INCLUDE_DIR rasqal.h
      PATHS
      ${redland_INCLUDE_DIRS}
      ${rasqal_INCLUDE_DIRS}
      /usr/X11/include
    )
    find_path(RAPTOR_INCLUDE_DIR raptor.h
      PATHS
      ${redland_INCLUDE_DIRS}
      ${raptor_INCLUDE_DIRS}
      /usr/X11/include
    )
  endif(WIN32)

  find_library(REDLAND_LIBRARIES NAMES rdf librdf
    PATHS
    ${redland_LIBRARY_DIRS}
  )
 
  find_library(RASQAL_LIBRARIES NAMES rasqal librasqal
    PATHS
    ${rasqal_LIBRARY_DIRS}
  )
 
  find_library(RAPTOR_LIBRARIES NAMES raptor libraptor
    PATHS
    ${raptor_LIBRARY_DIRS}
  )

  if (REDLAND_LIBRARIES AND REDLAND_INCLUDE_DIR AND RASQAL_INCLUDE_DIR AND RASQAL_LIBRARIES AND RAPTOR_LIBRARIES)
    set(REDLAND_FOUND TRUE)
  endif (REDLAND_LIBRARIES AND REDLAND_INCLUDE_DIR AND RASQAL_INCLUDE_DIR AND RASQAL_LIBRARIES AND RAPTOR_LIBRARIES)

  if (REDLAND_FOUND)
    set(REDLAND_DEFINITIONS ${redland_CFLAGS})
    if (NOT Redland_FIND_QUIETLY)
      message(STATUS "Found Redland ${REDLAND_VERSION}: libs - ${REDLAND_LIBRARIES}; includes - ${REDLAND_INCLUDE_DIR}")
      message(STATUS "Found Raptor ${RAPTOR_VERSION}: libs - ${RAPTOR_LIBRARIES}; includes - ${RAPTOR_INCLUDE_DIR}")
      message(STATUS "Found Rasqal ${RASQAL_VERSION}: libs - ${RASQAL_LIBRARIES}; includes - ${RASQAL_INCLUDE_DIR}")
    endif (NOT Redland_FIND_QUIETLY)
  else (REDLAND_FOUND)
    if (Redland_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find Redland")
    endif (Redland_FIND_REQUIRED)
  endif (REDLAND_FOUND)

#  mark_as_advanced(REDLAND_LIBRARIES)

#endif (REDLAND_LIBRARIES AND REDLAND_INCLUDE_DIR AND RASQAL_INCLUDE_DIR AND RASQAL_LIBRARIES AND RAPTOR_LIBRARIES)
