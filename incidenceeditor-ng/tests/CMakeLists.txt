# set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
# 
# include_directories( ../ )
# 
# set( conflictresolvertest_SRCS
#      conflictresolvertest.cpp )
# 
# kde4_add_unit_test(conflictresolvertest  ${conflictresolvertest_SRCS} )
# 
# target_link_libraries(
#   conflictresolvertest
#   ${QT_QTTEST_LIBRARY}
#   ${KDE4_KDEUI_LIBS}
#   ${KDEPIMLIBS_AKONADI_LIBS}
#   ${KDEPIMLIBS_KCAL_LIBS}
#   ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY}
#   incidenceeditorsng
#   akonadi-kcal_next
#   kdepimdbusinterfaces
# )
###########
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ../
)

MACRO(IE_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp)
    target_link_libraries(${_testname} kcalcore ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_AKONADI_KCAL_LIBRARY} kdgantt2 kcalutils kcalcore incidenceeditorsng akonadi-kcal_next kdepimdbusinterfaces)
  ENDFOREACH(_testname)
ENDMACRO(IE_UNIT_TESTS)

MACRO(IE_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_executable(${_testname} NOGUI TEST ${_testname}.cpp)
    target_link_libraries(${_testname} kcalcore ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS})
  ENDFOREACH(_testname)
ENDMACRO(IE_EXECUTABLE_TESTS)

IE_UNIT_TESTS(
  conflictresolvertest
  testfreebusyitemmodel
)