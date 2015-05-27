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
  add_executable( ${_name} ${_test} ${CMAKE_SOURCE_DIR}/messagecore/messagecore_debug.cpp ${CMAKE_SOURCE_DIR}/messagecore/autotests/util.cpp  testcsshelper.cpp )
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
    KF5::AkonadiNotes
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
add_messageviewer_unittest( noteedittest.cpp )
add_messageviewer_unittest( eventdatetimewidgettest.cpp )
add_messageviewer_unittest( messagedisplayformatattributetest.cpp )
add_messageviewer_unittest( gravatarconfigwidgettest.cpp )

# convenience macro to add qtest unit tests
macro(add_messageviewer_mailsourceviewbrowserwidget_unittest _source)
  set(_test ${_source} util.cpp)
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} ../widgets/mailsourceviewtextbrowserwidget.cpp ../findbar/findbarbase.cpp ../findbar/findbarsourceview.cpp )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(messageviewer-${_name})
  target_link_libraries( ${_name}
    Qt5::Test
    KF5::KIOCore
    Qt5::Gui
    Qt5::Widgets
    KF5::Mime
    KF5::AkonadiCore
    KF5::CalendarCore
    KF5::PimTextEdit
    pimcommon
  )
endmacro ()



add_messageviewer_mailsourceviewbrowserwidget_unittest( mailsourceviewtextbrowserwidgettest.cpp )
########### viewertest_gui ###############

set( messageviewer_converthtmltoplaintext_SRCS converthtmltoplaintexttest.cpp ../viewer/converthtmltoplaintext.cpp)
add_executable( messageviewer_converthtmltoplaintext ${messageviewer_converthtmltoplaintext_SRCS})
add_test(messageviewer_converthtmltoplaintext messageviewer_converthtmltoplaintext)
ecm_mark_as_test(messageviewer_converthtmltoplaintext)
target_link_libraries( messageviewer_converthtmltoplaintext Qt5::Test Grantlee5::TextDocument)



set(KDEPIMLIBS_RUN_ISOLATED_TESTS TRUE)
set(KDEPIMLIBS_RUN_SQLITE_ISOLATED_TESTS TRUE)

add_akonadi_isolated_test_advanced( viewertest.cpp "" "messageviewer")

