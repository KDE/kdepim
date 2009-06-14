# TODO: Remove ${KDE4_INCLUDES} and clean up headers/qt3 stuff.
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/../lib
	${KDE4_INCLUDES}
)

link_libraries( ${QT_QT3SUPPORT_LIBRARY} )

# We need Akonadi, and by extension Boost. But in particular
# we need the Boost includes.
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# libs
add_subdirectory(akonadibase)
add_subdirectory(base)

# Conduits
add_subdirectory(contacts)
add_subdirectory(calendar)
add_subdirectory(memofileconduit)
add_subdirectory(todo)

# Conduits disabled by default.
if(HAVE_TESTS_ENABLED)
	add_subdirectory(null)
	add_subdirectory(notepadconduit)
	add_subdirectory(popmail)

	if(QCA2_FOUND)
		add_subdirectory(keyringconduit)
	else(QCA2_FOUND)
		message(STATUS "QCA2 not found, keyringconduit disabled.")
	endif(QCA2_FOUND)
else(HAVE_TESTS_ENABLED)
	message(STATUS "Not a debug build, keyring and nullconduit disabled.")
endif(HAVE_TESTS_ENABLED)

add_subdirectory(timeconduit)

#add_subdirectory(docconduit)
#add_subdirectory(sysinfoconduit)

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

