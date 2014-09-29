macro(add_plaintexteditor_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(pimcommon-plaintexteditor-${_name})
  target_link_libraries( ${_name}
    pimcommon
    Qt5::Test
  )
endmacro ()

add_plaintexteditor_unittest( plaintexteditortest.cpp )
