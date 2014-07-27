set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ../
)

MACRO(IE_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    add_executable( ${_testname} ${_testname}.cpp modeltest.cpp )
    add_test( ${_testname} ${_testname} )
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname} Qt5::Test   KF5::AkonadiCore  kdgantt2 KF5::CalendarUtils KF5::CalendarCore incidenceeditorsng kdepimdbusinterfaces)
  ENDFOREACH(_testname)
ENDMACRO(IE_UNIT_TESTS)

MACRO(IE_EXECUTABLE_TESTS)
  FOREACH(_testname ${ARGN})
    add_executable(${_testname} NOGUI TEST ${_testname}.cpp modeltest.cpp)
    target_link_libraries(${_testname} KF5::CalendarCore Qt5::Test   incidenceeditorsng)
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

add_executable(ktimezonecomboboxtest ${ktimezonecomboboxtest_SRCS})
add_test(ktimezonecomboboxtest ktimezonecomboboxtest)
ecm_mark_as_test(ktimezonecomboboxtest)

target_link_libraries(
  ktimezonecomboboxtest
  KF5::CalendarCore
  Qt5::Test
  incidenceeditorsng
)

kde4_add_executable(testindividualmaildialog NOGUI TEST testindividualmaildialog.cpp ../individualmaildialog.cpp)
target_link_libraries(testindividualmaildialog
  KF5::CalendarCore
  Qt5::Test
)
