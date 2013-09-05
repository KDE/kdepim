set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  ${CMAKE_SOURCE_DIR}/messageviewer/
  ${CMAKE_BINARY_DIR}/messageviewer/
)

macro( add_scamdetection_test _source )
  set( _test ${_source} )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME messageviewer-${_name} ${_test} )
  target_link_libraries( ${_name} messageviewer ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTWEBKIT_LIBRARY} )
endmacro()

add_scamdetection_test(test_scamdetection.cpp)



set(scamdetection_gui_SRCS scamdetection_gui.cpp)
kde4_add_executable(scamdetection_gui TEST ${scamdetection_gui_SRCS})
target_link_libraries(scamdetection_gui
  ${QT_QTTEST_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${KDE4_KDEUI_LIBS}
  ${QT_QTWEBKIT_LIBRARY}
  messageviewer
)


