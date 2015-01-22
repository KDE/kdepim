set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/../../../ )


macro(kaddressbook_printing_grantlee _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(kaddressbookprintinggrantlee-${_name})
  target_link_libraries( ${_name}
    kaddressbookprivate
    Qt5::Test
    KF5::Contacts
  )
endmacro ()


kaddressbook_printing_grantlee(grantleeprinttest.cpp)
