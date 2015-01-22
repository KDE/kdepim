set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_SOURCE_DIR}/../../../ )

# Convenience macro to add unit tests.
macro( kaddressbook_printing_grantlee _source )
  set( _test ${_source}  )
  get_filename_component( _name ${_source} NAME_WE )
  kde4_add_unit_test( ${_name} TESTNAME kaddressbookprintinggrantlee-${_name} ${_test} )
  target_link_libraries( ${_name} kaddressbookprivate ${QT_QTTEST_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_AKONADI_LIBS} ${KDEPIMLIBS_KABC_LIBS} )
endmacro()

kaddressbook_printing_grantlee(grantleeprinttest.cpp)
