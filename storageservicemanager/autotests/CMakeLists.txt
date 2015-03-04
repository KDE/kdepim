set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# convenience macro to add qtest unit tests
macro(add_storageservice_unittest _source _additionalSource)
  set(_test ${_source} ${_additionalSource})
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

add_storageservice_unittest( storageservicenavigationbuttontest.cpp "../storageservicenavigationbuttons.cpp;../storageservicemanagerutil.cpp")
add_storageservice_unittest( storageserviceconfigurestoragewidgettest.cpp "../storageserviceconfigurestoragewidget.cpp")
