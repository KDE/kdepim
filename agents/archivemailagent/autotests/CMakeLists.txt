include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

# Convenience macro to add unit tests.
macro( archivemail_agent _source )
  set( _test ${_source} ../archivemailinfo.cpp)
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(archivemailagent-${_name})
  target_link_libraries( ${_name} Qt5::Test Qt5::Core mailcommon )
endmacro()

archivemail_agent(archivemailinfotest.cpp)
