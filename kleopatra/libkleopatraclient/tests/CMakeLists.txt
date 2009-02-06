set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_SOURCE_DIR}/kleopatra )

set( kleoclient_TESTS
  test_signencryptfilescommand
  test_decryptverifyfilescommand
  )

foreach( _kleoclient_test ${kleoclient_TESTS} )
  kde4_add_executable( "${_kleoclient_test}" TEST "${_kleoclient_test}.cpp" )
  target_link_libraries( "${_kleoclient_test}" kleopatraclientcore ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} )
endforeach( _kleoclient_test )

