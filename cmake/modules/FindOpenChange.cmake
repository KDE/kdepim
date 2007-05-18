# - Try to find the OpenChange MAPI library
# Once done this will define
#
#  LIBMAPI_FOUND - system has OpenChange MAPI library (libmapi)
#  LIBMAPI_INCLUDE_DIRS - the libmapi include directories
#  LIBMAPI_LIBRARIES - Required libmapi link libraries
#  LIBMAPI_DEFINITIONS - Compiler switches for libmapi
#
# Copyright (C) 2007 Brad Hards (bradh@frogmouth.net)
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the COPYING-CMAKE-SCRIPTS file in kdelibs/cmake/modules/

if (LIBMAPI_INCLUDE_DIRS AND LIBMAPI_LIBRARIES)

  # in cache already
  SET(LIBMAPI_FOUND TRUE)

else (LIBMAPI_INCLUDE_DIRS AND LIBMAPI_LIBRARIES)

  INCLUDE(UsePkgConfig)

  PKGCONFIG(libmapi _MAPIIncDir _MAPILinkDir _MAPILinkFlags _MAPICflags)

  set(LIBMAPI_DEFINITIONS ${_MAPICflags})
  set(LIBMAPI_INCLUDE_DIRS ${_MAPIIncDir})
  set(LIBMAPI_LIBRARIES ${_MAPILinkFlags})
  
  if (LIBMAPI_INCLUDE_DIRS AND LIBMAPI_LIBRARIES)
     set(LIBMAPI_FOUND TRUE)
  endif (LIBMAPI_INCLUDE_DIRS AND LIBMAPI_LIBRARIES)
  
  if (LIBMAPI_FOUND)
    if (NOT LIBMAPI_FIND_QUIETLY)
      message(STATUS "Found OpenChange MAPI library: ${LIBMAPI_LIBRARIES}")
    endif (NOT LIBMAPI_FIND_QUIETLY)
  else (LIBMAPI_FOUND)
    if (LIBMAPI_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find OpenChange MAPI library")
    endif (LIBMAPI_FIND_REQUIRED)
  endif (LIBMAPI_FOUND)
  
  MARK_AS_ADVANCED(LIBMAPI_INCLUDE_DIRS LIBMAPI_LIBRARIES LIBMAPI_DEFINITIONS)
  
endif (LIBMAPI_INCLUDE_DIRS AND LIBMAPI_LIBRARIES)
