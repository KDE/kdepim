include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

# Convenience macro to add unit tests.
macro( sendlater_agent _source )
  set( _test ${_source} ../sendlaterconfiguredialog.cpp ../sendlaterconfigurewidget.cpp)
  ki18n_wrap_ui(_test ../ui/sendlaterconfigurewidget.ui)
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(sendlatertest-${_name})
  target_link_libraries( ${_name} Qt5::Test KF5::AkonadiCore sendlater pimcommon)
endmacro()

sendlater_agent(sendlaterinfotest.cpp)
sendlater_agent(sendlaterconfiguredialogtest.cpp)
sendlater_agent(sendlaterconfigtest.cpp)

