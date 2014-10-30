include_directories(${CMAKE_SOURCE_DIR}/pimcommon/
)


macro(add_pimcommon_baloodebug_unittest _source)
  set(_test ${_source} )
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME pimcommon-baloodebug-${_name} ${_test} )
  target_link_libraries( ${_name}
    pimcommon
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
  )
endmacro ()

add_pimcommon_baloodebug_unittest(baloodebugdialogtest.cpp)
#add_pimcommon_baloodebug_unittest()
