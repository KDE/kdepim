add_subdirectory(bnf_selection)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# Convenience macro to add unit tests.
macro( add_mobileui_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME mobileui-${_name} ${_test} )
  target_link_libraries( ${_name} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} )
endmacro()

add_mobileui_test( rangemanagertest.cpp )
