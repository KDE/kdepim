INCLUDE(CheckCXXSourceCompiles)

set(CMAKE_INCLUDE_PATH "${MAL_BASE}/include")
FIND_PATH(MAL_INCLUDE_DIR libmal.h 
	${MAL_BASE}/include
	${MAL_BASE}/include/libmal
	/usr/include
	/usr/include/libmal
	/usr/local/include
	/usr/local/include/libmal
)
set(CMAKE_LIBRARY_PATH "${MAL_BASE}/lib")
FIND_LIBRARY(MAL_LIBRARY mal 
	${MAL_BASE}/lib
	/usr/lib
	/usr/lib/libmal
	/usr/local/lib
	/usr/local/lib/libmal
)

IF (NOT MAL_INCLUDE_DIR)
	MESSAGE(STATUS "Could not find libmal.h")
ELSE (NOT MAL_INCLUDE_DIR)
	MESSAGE(STATUS "Found libmal.h in ${MAL_INCLUDE_DIR}")
ENDIF (NOT MAL_INCLUDE_DIR)

IF (NOT MAL_LIBRARY)
	MESSAGE(STATUS "Could not find libmal")
ELSE (NOT MAL_LIBRARY)
	MESSAGE(STATUS "Found libmal in ${MAL_LIBRARY}")
ENDIF (NOT MAL_LIBRARY)

IF (MAL_INCLUDE_DIR AND MAL_LIBRARY)
	SET(MAL_FOUND TRUE)
ENDIF (MAL_INCLUDE_DIR AND MAL_LIBRARY)

IF (MAL_FOUND)
	SET(CMAKE_REQUIRED_INCLUDES ${MAL_INCLUDE_DIR} ${PILOTLINK_INCLUDE_DIR})
	CHECK_CXX_SOURCE_COMPILES("
#include <libmal.h>
#define LIBMAL_IS(a,b) ((LIBMAL_VERSION > a) || ((LIBMAL_VERSION == a) && ((LIBMAL_MAJOR > b) || (LIBMAL_MAJOR == b))))
#if !LIBMAL_IS(0,40)
#error \"Libmal version is < 0.40\"
#else
int main() { return 0; }
#endif
"
		MAL_VERSION_OK)
ENDIF (MAL_FOUND)

IF (NOT MAL_VERSION_OK)
	SET(MAL_FOUND FALSE)
	MESSAGE(STATUS "Found mal, but it's not at least version 0.40.")
ENDIF (NOT MAL_VERSION_OK)


IF (MAL_FOUND)
	MESSAGE(STATUS "Found mal: ${MAL_LIBRARY}")
ELSE (MAL_FOUND)
	MESSAGE(STATUS "Couldn't find acceptable mal version. Won't be able to build malconduit")
ENDIF (MAL_FOUND)

