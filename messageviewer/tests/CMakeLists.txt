set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/messageviewer/
  ${CMAKE_BINARY_DIR}/messageviewer/
  ${Boost_INCLUDE_DIRS}
)

########### htmlquotecolorertest ###############

set(htmlquotecolorertest_SRCS htmlquotecolorertest.cpp ../htmlquotecolorer.cpp)
kde4_add_unit_test(htmlquotecolorertest TESTNAME messageviewer-quotecolorertest ${htmlquotecolorertest_SRCS})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
target_link_libraries(htmlquotecolorertest
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  messageviewer
)

########### stringutiltest ###############

set(stringutiltest_SRCS stringutiltest.cpp)
add_definitions(-DKMAIL_UNITTESTS=YES)
kde4_add_unit_test(stringutiltest TESTNAME messageviewer-stringutiltest ${stringutiltest_SRCS})
target_link_libraries(stringutiltest
  messageviewer
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${KDE4_KIO_LIBS}
)

########### objecttreeparsertest ###############

set(objecttreeparsertest_SRCS objecttreeparsertest.cpp)
add_definitions(-DKMAIL_UNITTESTS=YES)
kde4_add_unit_test(objecttreeparsertest TESTNAME messageviewer-objecttreeparsertest ${objecttreeparsertest_SRCS})
target_link_libraries(objecttreeparsertest
  messageviewer
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QT3SUPPORT_LIBRARY}
  ${KDE4_KIO_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
)

