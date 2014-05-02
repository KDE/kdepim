set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ../
)

MACRO(IE_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp modeltest.cpp)
    target_link_libraries(${_testname} ${QT_QTTEST_LIBRARY}   KF5::AkonadiCore  kdgantt2 KF5::CalendarUtils KF5::CalendarCore incidenceeditorsng kdepimdbusinterfaces)
  ENDFOREACH(_testname)
ENDMACRO(IE_UNIT_TESTS)

MACRO(IE_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    add_executable(${_testname} NOGUI TEST ${_testname}.cpp modeltest.cpp)
    target_link_libraries(${_testname} KF5::CalendarCore ${QT_QTTEST_LIBRARY}   incidenceeditorsng)
  ENDFOREACH(_testname)
ENDMACRO(IE_EXECUTABLE_TESTS)

IE_UNIT_TESTS(
  conflictresolvertest
  testfreebusyitemmodel
  testfreeperiodmodel
  testfreebusyganttproxymodel
)

########### KTimeZoneComboBox unit test #############
set(ktimezonecomboboxtest_SRCS ktimezonecomboboxtest.cpp)

kde4_add_unit_test(ktimezonecomboboxtest TESTNAME ktimezonecomboboxtest ${ktimezonecomboboxtest_SRCS})

target_link_libraries(
  ktimezonecomboboxtest
  KF5::CalendarCore
  incidenceeditorsng
)

