include_directories(${CMAKE_SOURCE_DIR}/libksieve
        )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro( add_managesievejob_test _source )
  set( _test ${_source})
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(managejob-${_name})
  target_link_libraries( ${_name}  ksieveui Qt5::Test    )
endmacro()


add_managesievejob_test( parseuserjobtest.cpp )



