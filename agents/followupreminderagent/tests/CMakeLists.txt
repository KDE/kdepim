set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )


include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

# Convenience macro to add unit tests.
macro( followupreminder_agent _source )
  set( _test ${_source} ../followupreminderinfo.cpp )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME followupreminder-${_name} ${_test} )
  target_link_libraries( ${_name} Qt5::Test KF5::AkonadiCore )
endmacro()

followupreminder_agent(followupreminderinfotest.cpp)

