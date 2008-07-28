# TODO: Remove ${KDE4_INCLUDES} and clean up headers/qt3 stuff.
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../lib
	${KDE4_INCLUDES}
)

link_libraries( ${QT_QT3SUPPORT_LIBRARY} )

add_subdirectory(base)
add_subdirectory(contacts)
add_subdirectory(memofileconduit)

IF (QCA2_FOUND)
	add_subdirectory(keyringconduit)
ENDIF(QCA2_FOUND)

add_subdirectory(null)
add_subdirectory(timeconduit)


#add_subdirectory(docconduit)
add_subdirectory(notepadconduit)
add_subdirectory(popmail)
#add_subdirectory(sysinfoconduit)

FIND_PATH( HAVE_CALENDARLOCAL_H "kcal/calendarlocal.h" ${KDE4_INCLUDE_DIR} )

IF (HAVE_CALENDARLOCAL_H)
	add_subdirectory(vcalconduit)
ELSE (HAVE_CALENDARLOCAL_H)
	MESSAGE(STATUS "No KDE PIM development headers were found. Install it if you want to build it")
ENDIF (HAVE_CALENDARLOCAL_H)

###
#
# MAL seems to be broken, or the MAL API has changed somewhat
# since the last time that the conduit was compiled by the development
# team. Since we don't use the conduit it is disabled. 
#IF (MAL_FOUND)
#	MESSAGE(STATUS "MAL conduit will not be compiled, not ported yet.")
# 	add_subdirectory(malconduit)
#ELSE (MAL_FOUND)
#	MESSAGE(STATUS "Couldn't find mal. Won't be able to build malconduit")
#ENDIF (MAL_FOUND)

