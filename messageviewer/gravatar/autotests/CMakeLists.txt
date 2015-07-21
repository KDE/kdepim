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
macro(add_messageviewer_gravatar_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
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

add_messageviewer_gravatar_unittest( gravatarconfigwidgettest.cpp )
add_messageviewer_gravatar_unittest( gravatarconfiguresettingsdialogtest.cpp )
