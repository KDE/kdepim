include_directories(${CMAKE_SOURCE_DIR}/mailcommon
)

macro(add_mailcommon_widget_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(mailcommon-widget-${_name})

  target_link_libraries( ${_name}
    mailcommon
    Qt5::Test
    Qt5::Core
    Qt5::Gui
  )
endmacro ()

add_mailcommon_widget_unittest(redirectdialogtest.cpp)
