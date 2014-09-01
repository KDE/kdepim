set(sendlaterinfo_gui_SRCS sendlaterinfo_gui.cpp )
add_executable(sendlaterinfo_gui ${sendlaterinfo_gui_SRCS})
target_link_libraries(sendlaterinfo_gui
  sendlater
)


include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. )

remove_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )

# Convenience macro to add unit tests.
macro( sendlater_agent _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME sendlater-${_name} ${_test} )
  target_link_libraries( ${_name} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} sendlater)
endmacro()

sendlater_agent(sendlaterinfotest.cpp)

