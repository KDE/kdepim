set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
)
# convenience macro to add qtest unit tests
macro(add_storageservice_unittest _source)
  set(_test ${_source} ../storageservicenavigationbuttons.cpp ../storageservicemanagerutil.cpp)
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME storageservicemanager-${_name} ${_test} )
  target_link_libraries( ${_name}
    Qt5::Test
  )
endmacro ()

add_storageservice_unittest( storageservicenavigationbuttontest.cpp )
