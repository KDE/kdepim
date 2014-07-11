set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
)
# convenience macro to add qtest unit tests
macro(add_storageservice_unittest _source)
  set(_test ${_source} ../storageservicenavigationbuttons.cpp ../storageservicemanagerutil.cpp)
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(storageservicemanager-${_name})
  target_link_libraries( ${_name}
    Qt5::Test
    Qt5::Gui
    Qt5::Widgets
    KF5::I18n
    KF5::ConfigGui
  )
endmacro ()

add_storageservice_unittest( storageservicenavigationbuttontest.cpp )
