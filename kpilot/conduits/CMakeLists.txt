include_directories(
	${CMAKE_BINARY_DIR}/lib
	${CMAKE_SOURCE_DIR}/lib
	${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(abbrowserconduit)
# add_subdirectory(docconduit)
add_subdirectory(knotes)
add_subdirectory(memofileconduit)
add_subdirectory(notepadconduit)
add_subdirectory(null)
add_subdirectory(popmail)
add_subdirectory(sysinfoconduit)
add_subdirectory(timeconduit)
add_subdirectory(vcalconduit)

# MAL seems to be broken, or the MAL API has changed somewhat
# since the last time that the conduit was compiled by the development
# team. Since we don't use the conduit it is disabled. 
IF (MAL_FOUND)
	# add_subdirectory(malconduit)
	MESSAGE(STATUS "Although mal was found, the malconduit will not be compiled since it is unmaintained")
ELSE (MAL_FOUND)
	MESSAGE(STATUS "Couldn't find mal. Won't be able to build malconduit")
ENDIF (MAL_FOUND)
