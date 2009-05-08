set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
link_directories(${CMAKE_BINARY_DIR}/lib ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib ${CMAKE_CURRENT_BINARY_DIR}/../lib)

set(testactions_SRCS testactions.cc)
kde4_add_unit_test(testactions ${testactions_SRCS})
target_link_libraries(testactions kpilot ${QT_LIBRARIES} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

set(testconstants_SRCS testconstants.cc)
kde4_add_unit_test(testconstants ${testconstants_SRCS})
target_link_libraries(testconstants kpilot ${QT_LIBRARIES})

set(testcategories_SRCS testcategories.cc)
kde4_add_unit_test(testcategories ${testcategories_SRCS})
target_link_libraries(testcategories kpilot ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_LIBRARIES})

set(testaddresses_SRCS testaddress.cc)
kde4_add_unit_test(testaddresses ${testaddresses_SRCS})
target_link_libraries(testaddresses kpilot ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_LIBRARIES})

set(testdatebook_SRCS testdatebook.cc)
kde4_add_unit_test(testdatebook ${testdatebook_SRCS})
target_link_libraries(testdatebook kpilot ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_LIBRARIES})

#if (HAVE_CALENDARLOCAL_H)
#	set(importdatebook_SRCS importdatebook.cc)
#	set(exportdatebook_SRCS exportdatebook.cc)
#	set(mergecalendars_SRCS mergecalendars.cc)
#	kde4_add_unit_test(importdatebook ${importdatebook_SRCS})
#	kde4_add_unit_test(exportdatebook ${exportdatebook_SRCS})
#	kde4_add_unit_test(mergecalendars ${mergecalendars_SRCS})
#	target_link_libraries(importdatebook kpilot ${QT_LIBRARIES} ${KDEPIMLIBS_KCAL_LIBS})
#	target_link_libraries(exportdatebook kpilot ${QT_LIBRARIES} ${KDEPIMLIBS_KCAL_LIBS})
#	target_link_libraries(mergecalendars ${QT_LIBRARIES} ${KDEPIMLIBS_KCAL_LIBS})
#endif (HAVE_CALENDARLOCAL_H)

#set(importaddresses_SRCS importaddresses.cc)
#kde4_add_unit_test(importaddresses ${importaddresses_SRCS})
#target_link_libraries(importaddresses kpilot ${QT_LIBRARIES} ${KDEPIMLIBS_KABC_LIBS} kabc_file)

