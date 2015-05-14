
set( sieveedit_serversievesettings_source  serversievesettingstest.cpp ../serversievesettings.cpp ../sieveeditor_debug.cpp)
ki18n_wrap_ui(sieveedit_serversievesettings_source
    ../ui/serversievesettings.ui
)

add_executable( serversievesettingstest  ${sieveedit_serversievesettings_source})
add_test(serversievesettingstest serversievesettingstest)
ecm_mark_as_test(serversievesettingstest)
target_link_libraries( serversievesettingstest Qt5::Test Qt5::Widgets KF5::MailTransport KF5::I18n KF5::ConfigGui)

#REACTIVATE
if (false)
set( sieveeditorutiltest_source sieveeditorutiltest.cpp ../sieveeditorutil.cpp ../sieveserversettings.cpp)
add_executable( sieveeditorutiltest ${sieveeditorutiltest_source})
add_test(sieveeditorutiltest sieveeditorutiltest)
ecm_mark_as_test(sieveeditorutiltest)
target_link_libraries( sieveeditorutiltest Qt5::Test )
endif()

# convenience macro to add qtest unit tests
macro(add_sieveserver_unittest _source _additionalSource)
  set(_test ${_source} ${_additionalSource})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(sieveserver-${_name})
  target_link_libraries( ${_name}
    Qt5::Test
    Qt5::Core
    Qt5::Widgets 
    KF5::I18n  
  )
endmacro ()

add_sieveserver_unittest( sieveeditorconfigureserverpagetest.cpp "../sieveeditorconfigureserverpage.cpp")
