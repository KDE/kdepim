include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_BINARY_DIR}/../../../lib
)

MACRO( RECORD_CONDUIT_UNIT_TESTS )
	FOREACH( _testname ${ARGN} )
		kde4_add_unit_test( ${_testname} ${_testname}.cc ${${_testname}_SRCS} )
		target_link_libraries( ${_testname}
			kpilot_conduit_base
			testlib
			${QT_QTTEST_LIBRARY} 
			${QT_QTCORE_LIBRARY} 
			${QT_QTGUI_LIBRARY} 
			${KDE4_KDECORE_LIBS} 
		)
	ENDFOREACH( _testname )
ENDMACRO( RECORD_CONDUIT_UNIT_TESTS )

MACRO( RECORD_CONDUIT_EXECUTABLE_TESTS )
	FOREACH( _testname ${ARGN} )
		kde4_add_executable( TEST ${_testname} ${_testname}.cc )
		target_link_libraries( ${_testname} 
			kpilot_conduit_base
			testlib
			${QT_QTTEST_LIBRARY} 
		)
	ENDFOREACH( _testname )
ENDMACRO( RECORD_CONDUIT_EXECUTABLE_TESTS )

set(testlib_SRCS
	testrecord.cc
	testhhrecord.cc
	testdataproxy.cc
	testhhdataproxy.cc
	testrecordconduit.cc
)

kde4_add_library(testlib SHARED ${testlib_SRCS})
target_link_libraries(testlib kpilot_conduit_base kpilot ${KDE4_KDEUI_LIBS})

# NOTE: To run individual test with verbose output do ctest -V -R <testname>

RECORD_CONDUIT_UNIT_TESTS(
	idmappingxmlsourcetest
	idmappingtest
	dataproxytest
	rchotsynchhtest
	rcfullsynchhtest
	rcfirstsynctest
	rccopyhhtopctest
	rccopypctohhtest
	categoryhotsynctest
)
