
include_directories( ${CMAKE_SOURCE_DIR}/knode ${KDE4_INCLUDES} )


set( knodetest_SRCS
  knodetest.cpp
)

kde4_add_unit_test( knodetest
  TESTNAME knode-test
  ${knodetest_SRCS}
)

target_link_libraries( knodetest
  knodecommon
  ${QT_QTCORE_LIBRARY}
  ${QT_QTTEST_LIBRARY}
)
