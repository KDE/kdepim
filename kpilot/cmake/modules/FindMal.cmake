set(CMAKE_INCLUDE_PATH "${MAL_BASE}/include")
FIND_PATH(MAL_INCLUDE_DIR libmal.h 
	/usr/include
	/usr/include/libmal
	/usr/local/include
	/usr/local/include/libmal
)
set(CMAKE_LIBRARY_PATH "${MAL_BASE}/lib")
FIND_LIBRARY(MAL_LIBRARY mal 
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
	MESSAGE(STATUS "Found mal: ${MAL_LIBRARY}")
ELSE (MAL_FOUND)
	MESSAGE(STATUS "Couldn't find mal. Won't be able to build malconduit")
ENDIF (MAL_FOUND)

