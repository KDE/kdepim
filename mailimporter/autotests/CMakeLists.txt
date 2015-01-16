macro(mailimporter_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(mailimporter-${_name})
  target_link_libraries( ${_name}
    mailimporter
    Qt5::Test
    Qt5::Core
    Qt5::Gui
  )
endmacro ()

mailimporter_unittest(filterthunderbirdtest.cpp)
mailimporter_unittest(filtericedovetest.cpp)

