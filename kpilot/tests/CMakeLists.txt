link_directories(${CMAKE_BINARY_DIR}/lib ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/lib ${CMAKE_BINARY_DIR}/lib)

# Tests don't need to go into toplevel/bin, they are fine in the current dir.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# This can be used for finding data files in the source dir
add_definitions( -DKDESRCDIR=\\"${CMAKE_CURRENT_SOURCE_DIR}\\" )


set(testconstants_SRCS testconstants.cc)
kde3_add_executable(testconstants ${testconstants_SRCS})
target_link_libraries(testconstants kpilot ${QT_LIBRARIES})
add_test(testconstants ${EXECUTABLE_OUTPUT_PATH}/testconstants)

set(testcategories_SRCS testcategories.cc)
kde3_add_executable(testcategories ${testcategories_SRCS})
target_link_libraries(testcategories kpilot ${QT_LIBRARIES})
add_test(testcategories ${EXECUTABLE_OUTPUT_PATH}/testcategories)

set(testaddresses_SRCS testaddress.cc)
kde3_add_executable(testaddresses ${testaddresses_SRCS})
target_link_libraries(testaddresses kpilot ${QT_LIBRARIES})
add_test(testaddresses ${EXECUTABLE_OUTPUT_PATH}/testaddresses)

set(testdatebook_SRCS testdatebook.cc)
kde3_add_executable(testdatebook ${testdatebook_SRCS})
target_link_libraries(testdatebook kpilot ${QT_LIBRARIES})
add_test(testdatebook ${EXECUTABLE_OUTPUT_PATH}/testdatebook)

set(testidmapper_SRCS testidmapper.cc)
kde3_add_executable(testidmapper ${testidmapper_SRCS})
target_link_libraries(testidmapper kpilot ${QT_LIBRARIES})
add_test(testidmapper ${EXECUTABLE_OUTPUT_PATH}/testidmapper)

if (HAVE_CALENDARLOCAL_H)
	set(importdatebook_SRCS importdatebook.cc)
	set(exportdatebook_SRCS exportdatebook.cc)
	set(mergecalendars_SRCS mergecalendars.cc)
	kde3_add_executable(importdatebook ${importdatebook_SRCS})
	kde3_add_executable(exportdatebook ${exportdatebook_SRCS})
	kde3_add_executable(mergecalendars ${mergecalendars_SRCS})
	target_link_libraries(importdatebook kpilot ${QT_LIBRARIES} kcal)
	target_link_libraries(exportdatebook kpilot ${QT_LIBRARIES} kcal)
	target_link_libraries(mergecalendars ${QT_LIBRARIES} kcal)
	add_test(importdatebook ${EXECUTABLE_OUTPUT_PATH}/importdatebook)
	add_test(exportdatebook ${EXECUTABLE_OUTPUT_PATH}/exportdatebook)
	add_test(mergecalendars ${EXECUTABLE_OUTPUT_PATH}/mergecalendars)
endif (HAVE_CALENDARLOCAL_H)

set(importaddresses_SRCS importaddresses.cc)
kde3_add_executable(importaddresses ${importaddresses_SRCS})
target_link_libraries(importaddresses kpilot ${QT_LIBRARIES} kabc kabc_file)
add_test(importaddresses ${EXECUTABLE_OUTPUT_PATH}/importaddresses)

