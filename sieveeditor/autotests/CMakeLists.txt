
set( sieveedit_serversievesettings_source  serversievesettingstest.cpp ../serversievesettings.cpp)
kde4_add_ui_files(sieveedit_serversievesettings_source
    ../ui/serversievesettings.ui
)

kde4_add_unit_test( serversievesettingstest  ${sieveedit_serversievesettings_source})
target_link_libraries( serversievesettingstest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})


set( sieveeditorutiltest_source sieveeditorutiltest.cpp ../sieveeditorutil.cpp ../sieveserversettings.cpp)
kde4_add_unit_test( sieveeditorutiltest ${sieveeditorutiltest_source})
target_link_libraries( sieveeditorutiltest ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS})


# convenience macro to add qtest unit tests
macro(add_sieveserver_unittest _source _additionalSource)
  set(_test ${_source} ${_additionalSource})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME sieveserver-${_name} ${_test} )
  target_link_libraries( ${_name}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
  )
endmacro ()

add_sieveserver_unittest( sieveeditorconfigureserverpagetest.cpp "../sieveeditorconfigureserverpage.cpp")
