set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )


include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

# Convenience macro to add unit tests.
macro( followupreminder_agent _source )
  set( _test ${_source} ../followupreminderinfodialog.cpp ../followupreminderinfowidget.cpp ../jobs/followupremindershowmessagejob.cpp ../followupremindernoanswerdialog.cpp)
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(followupreminder-${_name})
  target_link_libraries( ${_name} Qt5::Test KF5::AkonadiCore followupreminder)
endmacro()

followupreminder_agent(followupreminderinfotest.cpp)
followupreminder_agent(followupreminderinfodialogtest.cpp)
followupreminder_agent(followupremindernoanswerdialogtest.cpp)
followupreminder_agent(followupreminderconfigtest.cpp)
