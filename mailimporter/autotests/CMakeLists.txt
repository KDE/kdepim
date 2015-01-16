macro(mailimporter_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME mailimporter-${_name} ${_test} )
  target_link_libraries( ${_name}
    mailimporter
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDEUI_LIBS}
  )
endmacro ()

mailimporter_unittest(filterthunderbirdtest.cpp)
mailimporter_unittest(filtericedovetest.cpp)
mailimporter_unittest(filterclawsmailtest.cpp)
mailimporter_unittest(filtersylpheedtest.cpp)

