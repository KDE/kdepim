
macro(add_pimcommon_baloodebug_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(pimcommon-baloodebug-${_name})
  target_link_libraries( ${_name}
    pimcommon
    Qt5::Test
    Qt5::Core
    Qt5::Gui
    KF5::KDELibs4Support
    
  )
endmacro ()

add_pimcommon_baloodebug_unittest(baloodebugdialogtest.cpp)
add_pimcommon_baloodebug_unittest(baloodebugwidgettest.cpp)
