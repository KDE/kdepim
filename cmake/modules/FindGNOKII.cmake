# - Try to find the gnokii directory library
# Once done this will define
#
#  GNOKII_FOUND - system has GNOKII
#  GNOKII_INCLUDE_DIR - the GNOKII include directory
#  GNOKII_LIBRARIES - The libraries needed to use GNOKII

if (GNOKII_INCLUDE_DIR)
  # Already in cache, be silent
  set(GNOKII_FIND_QUIETLY TRUE)
endif (GNOKII_INCLUDE_DIR)

FIND_PATH(GNOKII_INCLUDE_DIR gnokii.h
   /usr/include
   /usr/local/include
)

FIND_LIBRARY(GNOKII_LIBRARIES NAMES gnokii
   PATHS
   /usr/lib
   /usr/local/lib
)


if (GNOKII_INCLUDE_DIR AND GNOKII_LIBRARIES)
   set(GNOKII_FOUND TRUE)
endif (GNOKII_INCLUDE_DIR AND GNOKII_LIBRARIES)


if (GNOKII_FOUND)
   if (NOT GNOKII_FIND_QUIETLY)
      message(STATUS "Found gnokii: ${GNOKII_LIBRARIES}")
   endif (NOT GNOKII_FIND_QUIETLY)
else(GNOKII_FOUND)
   if (GNOKII_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find gnokii")
   endif (GNOKII_FIND_REQUIRED)
endif (GNOKII_FOUND)

MARK_AS_ADVANCED(GNOKII_INCLUDE_DIR GNOKII_LIBRARIES)
