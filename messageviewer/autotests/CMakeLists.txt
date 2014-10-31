set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
add_definitions( -DMAIL_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/messageviewer/
  ${CMAKE_BINARY_DIR}/messageviewer/
  ${Boost_INCLUDE_DIRS}
)
include ( ${CMAKE_SOURCE_DIR}/cmake/modules/kde4_handle_crypto_rpath_for_executable.cmake )


add_definitions(-DKMAIL_UNITTESTS=YES)

# convenience macro to add qtest unit tests
macro(add_messageviewer_unittest _source)
  set(_test ${_source} util.cpp)
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} ${CMAKE_SOURCE_DIR}/messagecore/autotests/util.cpp  testcsshelper.cpp)
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(messageviewer-${_name})
  target_link_libraries( ${_name}
    messageviewer
    kleo
    KF5::QGpgme
    Qt5::Test
    Qt5::WebKitWidgets
    KF5::KIOCore
    KF5::Mime
    KF5::AkonadiCore
    KF5::CalendarCore
  )
endmacro ()

add_messageviewer_unittest( htmlquotecolorertest.cpp )
add_messageviewer_unittest( objecttreeparsertest.cpp )
kde4_handle_crypto_rpath_for_executable( objecttreeparsertest )
add_messageviewer_unittest( rendertest.cpp )
kde4_handle_crypto_rpath_for_executable( rendertest )
add_messageviewer_unittest( unencryptedmessagetest.cpp )
kde4_handle_crypto_rpath_for_executable( unencryptedmessagetest )
add_messageviewer_unittest( todoedittest.cpp )
add_messageviewer_unittest( eventedittest.cpp )
add_messageviewer_unittest( eventdatetimewidgettest.cpp )
add_messageviewer_unittest( mailsourceviewtextbrowserwidgettest.cpp )
########### viewertest_gui ###############


set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced( viewertest.cpp "" "messageviewer")

