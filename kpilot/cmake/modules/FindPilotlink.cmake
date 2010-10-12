INCLUDE(CheckCXXSourceCompiles)

set(CMAKE_INCLUDE_PATH "${PILOTLINK_BASE}/include")
FIND_PATH(PILOTLINK_INCLUDE_DIR pi-dlp.h 
	/usr/include
	/usr/include/libpisock
	/usr/local/include
	)
set(CMAKE_LIBRARY_PATH "${PILOTLINK_BASE}/lib")
FIND_LIBRARY(PILOTLINK_LIBRARY pisock 
	/usr/lib
	/usr/local/lib
	)

IF (NOT PILOTLINK_INCLUDE_DIR)
	MESSAGE(STATUS "Could not find pilot-link (pi-dlp.h)")
ELSE (NOT PILOTLINK_INCLUDE_DIR)
	MESSAGE(STATUS "Found pilot-link includes in  ${PILOTLINK_INCLUDE_DIR}")
ENDIF (NOT PILOTLINK_INCLUDE_DIR)

IF (NOT PILOTLINK_LIBRARY)
	MESSAGE(STATUS "Could not find pilot-link (libpisock.so)")
ELSE (NOT PILOTLINK_LIBRARY)
	MESSAGE(STATUS "Found pilot-link libraries in ${PILOTLINK_LIBRARY}")
ENDIF (NOT PILOTLINK_LIBRARY)

IF (PILOTLINK_INCLUDE_DIR AND PILOTLINK_LIBRARY)
	SET(PILOTLINK_FOUND TRUE)
ENDIF (PILOTLINK_INCLUDE_DIR AND PILOTLINK_LIBRARY)

IF (PILOTLINK_FOUND)
	SET(CMAKE_REQUIRED_INCLUDES ${PILOTLINK_INCLUDE_DIR})
	CHECK_CXX_SOURCE_COMPILES("
#include <pi-version.h>
#if !PILOT_LINK_IS(0,12,0)
#error \"Pilot-link version is < 0.12.0\"
#else
int main() { return 0; }
#endif
"
		PILOTLINK_VERSION_OK)
ENDIF (PILOTLINK_FOUND)

IF (NOT PILOTLINK_VERSION_OK)
	SET(PILOTLINK_FOUND FALSE)
ENDIF (NOT PILOTLINK_VERSION_OK)


IF (NOT PILOTLINK_FOUND)
	IF (Pilotlink_FIND_REQUIRED)
		MESSAGE(STATUS "KPilot requires pilot-link 0.12.0 or later. Pilot-link is available from pilot-link.org and is packaged by most distributions. Remember to install the development package with the compilation headers as well.")
		MESSAGE(FATAL_ERROR "Could not find pilot-link.")
	ENDIF (Pilotlink_FIND_REQUIRED)
ENDIF (NOT PILOTLINK_FOUND)

