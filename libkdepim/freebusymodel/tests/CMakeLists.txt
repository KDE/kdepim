set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
 ../
)

MACRO(IE_UNIT_TESTS)
  FOREACH(_testname ${ARGN})
    kde4_add_unit_test(${_testname} NOGUI ${_testname}.cpp modeltest.cpp)
    target_link_libraries(${_testname}
                          ${QT_QTTEST_LIBRARY}
                          ${KDEPIMLIBS_AKONADI_LIBS}
                          ${KDEPIMLIBS_KCALUTILS_LIBS}
                          ${KDEPIMLIBS_KCALCORE_LIBS}
                          kdepim)
  ENDFOREACH(_testname)
ENDMACRO(IE_UNIT_TESTS)

IE_UNIT_TESTS(
  testfreeperiodmodel
  testfreebusyitemmodel
)