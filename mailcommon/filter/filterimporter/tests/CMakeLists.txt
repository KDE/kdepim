set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# Convenience macro to add messagecomposer unit tests.
macro( add_filterimport_test _source )
  set( _test ${_source} filtertestkernel.cpp)
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME filterimport-${_name} ${_test} )
  target_link_libraries( ${_name}  mailcommon messagecomposer ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KPIMIDENTITIES_LIBS})
endmacro()


add_filterimport_test( filterimportclawmailtest.cpp )
add_filterimport_test( filterimportthunderbirdtest.cpp )
