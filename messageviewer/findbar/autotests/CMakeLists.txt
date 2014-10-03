include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer
  ${CMAKE_BINARY_DIR}/messageviewer
)

# convenience macro to add qtest unit tests
macro(add_messageviewer_findbarunittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} ../findbarbase.cpp)
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(messageviewer-${_name})
  target_link_libraries( ${_name}
    Qt5::Test
    Qt5::Widgets
    Qt5::Gui
    KF5::Completion 
    KF5::I18n
    KF5::ConfigWidgets
  )
endmacro ()


add_messageviewer_findbarunittest( findbarbasetest.cpp )
