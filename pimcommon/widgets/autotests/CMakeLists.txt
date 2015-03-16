include_directories(
  ${CMAKE_SOURCE_DIR}/pimcommon/
  ${CMAKE_BINARY_DIR}/pimcommon/
)

macro(add_pimcommon_widget_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(pimcommon-widget-${_name})
  target_link_libraries( ${_name}
    pimcommon
    Qt5::Test
    Qt5::Core
    Qt5::Gui
  )
endmacro ()

add_pimcommon_widget_unittest(slidecontainertest.cpp)

set( pimcommon_kactionmenuchangecasetest_source kactionmenuchangecasetest.cpp ../kactionmenuchangecase.cpp)
add_executable( kactionmenuchangecasetest ${pimcommon_kactionmenuchangecasetest_source})
add_test(kactionmenuchangecasetest kactionmenuchangecasetest)
ecm_mark_as_test(kactionmenuchangecasetest)
target_link_libraries( kactionmenuchangecasetest Qt5::Test KF5::WidgetsAddons KF5::I18n KF5::XmlGui)


