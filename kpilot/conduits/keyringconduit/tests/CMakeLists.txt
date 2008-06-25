include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

macro( keyring_conduit_unit_tests )
	foreach( _testname ${ARGN} )
		kde4_add_unit_test( ${_testname} ${_testname}.cc ${${_testname}_SRCS} )
		
		target_link_libraries( ${_testname}
			${QT_QTTEST_LIBRARY}
			keyring_testlib
		)
	endforeach( _testname )
endmacro( keyring_conduit_unit_tests )

macro( record_conduit_executable_tests )
	foreach( _testname ${ARGN} )
		kde4_add_executable( TEST ${_testname} ${_testname}.cc )
		target_link_libraries( ${_testname} 
			kpilot_conduit_keyring_test
			${QT_QTTEST_LIBRARY} 
		)
	endforeach( _testname )
endmacro( record_conduit_executable_tests )

set( keyring_testlib_SRCS
	../pi-keyring.cc
	../keyringsetup.cc
	../keyringhhrecord.cc
	../keyringhhdataproxy.cc
	../keyringconduit.cc
	../keyringfactory.cc
	testkeyringconduit.cc
)

set(keyring_KCFGS
	../keyringsettings.kcfgc
)

kde4_add_kcfg_files( keyring_testlib_SRCS ${keyring_KCFGS} )
kde4_add_library( keyring_testlib SHARED ${keyring_testlib_SRCS} )

target_link_libraries( keyring_testlib
	${QT_LIBRARIES}
	${QCA2_LIBRARIES}
	kpilot_conduit_base
	kpilot
)

# NOTE: To run individual test with verbose output do ctest -V -R <testname>
keyring_conduit_unit_tests (
	keyringcategorysynctest
)
