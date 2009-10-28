set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer/
  ${Boost_INCLUDE_DIRS}
)

########### htmlquotecolorertest ###############
set(htmlquotecolorertest_SRCS htmlquotecolorertest.cpp ../htmlquotecolorer.cpp)
kde4_add_unit_test(htmlquotecolorertest TESTNAME kmail-quotecolorertest ${htmlquotecolorertest_SRCS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
target_link_libraries(htmlquotecolorertest
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KHTML_LIBRARY}
  messageviewer
)

########### stringutiltest ###############

set(stringutiltest_SRCS stringutiltest.cpp ../stringutil.cpp)
add_definitions(-DKMAIL_UNITTESTS=YES)
kde4_add_unit_test(stringutiltest TESTNAME kmail-stringutiltest ${stringutiltest_SRCS})
target_link_libraries(stringutiltest messageviewer
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KIO_LIBS}
)


