add_subdirectory(bnf_selection)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# Convenience macro to add unit tests.
macro( add_mobileui_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(mobileui-${_name})
  target_link_libraries( ${_name} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )
endmacro()

add_mobileui_test( rangemanagertest.cpp )
