set( calendarsupporttests_SRCS historytest.cpp )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

kde4_add_unit_test( calendarsupporttests
  TESTNAME historytest
  ${calendarsupporttests_SRCS}
)

target_link_libraries( calendarsupporttests
  calendarsupport
  ${KDE4_KDECORE_LIBS}
  ${QT_QTTEST_LIBRARY}
)
