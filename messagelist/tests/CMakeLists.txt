set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

# Convenience macro to add unit tests.
macro( add_messagelist_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME messagelist-${_name} ${_test} )
  target_link_libraries( ${_name} messagecore messagelist ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} )
endmacro()

add_messagelist_test(itemsizetest.cpp)
add_messagelist_test(quicksearchlinetest.cpp)
