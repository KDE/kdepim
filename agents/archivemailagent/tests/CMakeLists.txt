include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

# Convenience macro to add unit tests.
macro( archivemail_agent _source )
  set( _test ${_source} ../archivemailinfo.cpp)
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME archivemailagent-${_name} ${_test} )
  target_link_libraries( ${_name} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} )
endmacro()

archivemail_agent(archivemailinfotest.cpp)
