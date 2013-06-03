set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
    ${CMAKE_SOURCE_DIR}/knode
    ${CMAKE_SOURCE_DIR}/libkdepim/
    ${KDE4_INCLUDES}
    ${Boost_INCLUDE_DIR}
    )


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
