# Convenience macro to add kleo unit tests.
macro( add_kleo_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  ecm_mark_as_test(kleo-${_name})
  target_link_libraries(${_name} KF5::Libkleo Qt5::Widgets KF5::CoreAddons KF5::I18n KF5::WidgetsAddons KF5::IconThemes)
endmacro()


add_kleo_test(test_kdhorizontalline.cpp)
add_kleo_test(test_jobs.cpp)
add_kleo_test(test_gnupgprocessbase.cpp)
add_kleo_test(test_cryptoconfig.cpp)
add_kleo_test(test_keyselectiondialog.cpp)
add_kleo_test(test_keygen.cpp)
add_kleo_test(test_keylister.cpp)
