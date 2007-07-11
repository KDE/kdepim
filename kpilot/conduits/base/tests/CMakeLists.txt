include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

MACRO( RECORD_CONDUIT_UNIT_TESTS )
	FOREACH( _testname ${ARGN} )
		kde4_add_unit_test( ${_testname} ${_testname}.cc )
		target_link_libraries( ${_testname} conduit_base ${QT_QTTEST_LIBRARY} )
	ENDFOREACH( _testname )
ENDMACRO( RECORD_CONDUIT_UNIT_TESTS )

MACRO( RECORD_CONDUIT_EXECUTABLE_TESTS )
	FOREACH( _testname ${ARGN} )
		kde4_add_test_executable( ${_testname} ${_testname}.cc )
		target_link_libraries( ${_testname} conduit_base ${QT_QTTEST_LIBRARY} )
	ENDFOREACH( _testname )
ENDMACRO( RECORD_CONDUIT_EXECUTABLE_TESTS )

RECORD_CONDUIT_EXECUTABLE_TESTS(
	idmappingxmlsourcetest
	idmappingtest
)

RECORD_CONDUIT_UNIT_TESTS (
	idmappingxmlsourcetest
	idmappingtest
)
