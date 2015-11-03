set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

macro( add_sievescriptdebugger_test _source )
  set( _test ${_source})
  get_filename_component( _name ${_source} NAME_WE )
  add_executable( ${_name} ${_test} )
  add_test( ${_name} ${_name} )
  ecm_mark_as_test(sievescriptdebugger-${_name})
  target_link_libraries( ${_name}  KF5::KSieveUi Qt5::Test KF5::Completion KF5::KIOWidgets KF5::PimTextEdit)
endmacro()


add_sievescriptdebugger_test( sievescriptdebuggerdialogtest.cpp )
add_sievescriptdebugger_test( sievescriptdebuggerwidgettest.cpp )
add_sievescriptdebugger_test( sievescriptdebuggerfrontendwidgettest.cpp )
add_sievescriptdebugger_test( sievescriptdebuggerwarningtest.cpp )
