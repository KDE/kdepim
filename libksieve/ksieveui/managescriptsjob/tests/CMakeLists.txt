include_directories(${CMAKE_SOURCE_DIR}/libksieve
        )

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro( add_managesievejob_test _source )
  set( _test ${_source})
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME managejob-${_name} ${_test} )
  target_link_libraries( ${_name}  ksieveui ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} )
endmacro()


add_managesievejob_test( parseuserjobtest.cpp )



