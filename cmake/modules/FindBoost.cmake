# - Try to find the boost directory library
# Once done this will define
#
#  BOOST_FOUND - system has BOOST
#  BOOST_INCLUDE_DIR - the BOOST include directory
#  BOOST_LIBRARIES - The libraries needed to use BOOST

if (BOOST_INCLUDE_DIR)
  # Already in cache, be silent
  set(BOOST_FIND_QUIETLY TRUE)
endif (BOOST_INCLUDE_DIR)

FIND_PATH(BOOST_INCLUDE_DIR boost/format.hpp
   /usr/include
   /usr/local/include
)

#FIND_LIBRARY(BOOST_LIBRARIES NAMES boost_iostreams
#   PATHS
#   /usr/lib
#   /usr/local/lib
#)


if (BOOST_INCLUDE_DIR)
   set(BOOST_FOUND TRUE)
endif (BOOST_INCLUDE_DIR)


if (BOOST_FOUND)
   if (NOT BOOST_FIND_QUIETLY)
      message(STATUS "Found boost: ${BOOST_INCLUDE_DIR}")
   endif (NOT BOOST_FIND_QUIETLY)
endif (BOOST_FOUND)

MARK_AS_ADVANCED(BOOST_INCLUDE_DIR BOOST_LIBRARIES)
