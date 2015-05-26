set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ../
)

MACRO(IE_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp modeltest.cpp)
    add_test(libkdepim_${_testname} ${_testname})
    ecm_mark_as_test(libkdepim_${_testname})
    ecm_mark_nongui_executable(${_testname})
    target_link_libraries(${_testname}
                          Qt5::Test
                          KF5::AkonadiCore
                          KF5::CalendarUtils
                          KF5::CalendarCore
                          kdepim)
  ENDFOREACH(_testname)
ENDMACRO(IE_UNIT_TESTS)

IE_UNIT_TESTS(
  testfreeperiodmodel
  testfreebusyitemmodel
)
