set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
add_definitions( -DMAIL_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"" )
add_definitions( -DGNUPGHOME="\\"${CMAKE_BINARY_DIR}/messagecore/tests/gnupg_home\\"" )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/messageviewer/
  ${CMAKE_BINARY_DIR}/messageviewer/
  ${Boost_INCLUDE_DIRS}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
add_definitions(-DKMAIL_UNITTESTS=YES)

# convenience macro to add qtest unit tests
macro(add_messageviewer_unittest _source)
  set(_test ${_source} util.cpp)
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME messageviewer-${_name} ${_test} ${CMAKE_SOURCE_DIR}/messagecore/tests/util.cpp)
  target_link_libraries( ${_name}
    messageviewer
    kleo
    ${QGPGME_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
  )
endmacro ()

add_messageviewer_unittest( htmlquotecolorertest.cpp )
add_messageviewer_unittest( objecttreeparsertest.cpp )
add_messageviewer_unittest( rendertest.cpp )
add_messageviewer_unittest( unencryptedmessagetest.cpp )


########### viewertest_gui ###############

set(viewertest_gui_SRCS viewertest_gui.cpp)
kde4_add_executable(viewertest_gui TEST ${viewertest_gui_SRCS})
target_link_libraries(viewertest_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  ${KDE4_KDECORE_LIBS}
  ${KDE4_KDEUI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  messageviewer
)
