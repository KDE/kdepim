include(CheckIncludeFiles)
include(CheckFunctionExists)

check_include_files( stdint.h HAVE_STDINT_H )
check_include_files( alloca.h HAVE_ALLOCA_H )
check_include_files( "sys/time.h" HAVE_SYS_TIME_H )
check_include_files( "sys/stat.h" HAVE_SYS_STAT_H )
check_function_exists( cfsetspeed HAVE_CFSETSPEED )
check_function_exists( strdup HAVE_STRDUP )
check_function_exists( setenv HAVE_SETENV )
check_function_exists( unsetenv HAVE_UNSETENV )
check_function_exists( usleep HAVE_USLEEP )
check_function_exists( random HAVE_RANDOM )
check_function_exists( putenv HAVE_PUTENV )
check_function_exists( seteuid HAVE_SETEUID )
check_function_exists( mkstemps HAVE_MKSTEMPS )
check_function_exists( mkstemp HAVE_MKSTEMP )
check_function_exists( mkdtemp HAVE_MKDTEMP )
check_function_exists( revoke HAVE_REVOKE )
check_function_exists( strlcpy HAVE_STRLCPY )
check_function_exists( strlcat HAVE_STRLCAT )
check_function_exists( inet_aton HAVE_INET_ATON )

configure_file(
	${CMAKE_SOURCE_DIR}/config.h.cmake
	${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(lib_SRCS
	options.cc
	plugin.cc
	syncAction.cc
	actions.cc
	actionQueue.cc
	idmapping.cc
	idmapperxml.cc
	idmapper.cc
	kpilotlink.cc
	kpilotdevicelink.cc
	kpilotlocallink.cc
	pilot.cc
	pilotAppInfo.cc
	pilotRecord.cc
	pilotDatabase.cc
	pilotLocalDatabase.cc
	pilotSerialDatabase.cc
	pilotMemo.cc
	pilotAddress.cc
	pilotDateEntry.cc
	pilotTodoEntry.cc
)

kde3_automoc(${lib_SRCS})
kde3_add_kcfg_files(lib_SRCS kpilotlibSettings.kcfgc)
add_library(kpilot SHARED ${lib_SRCS})
target_link_libraries(kpilot ${PILOTLINK_LIBRARY} ${QT_LIBRARIES} kdeui kio)
kpilot_rpath(kpilot)

#---------- INSTALL -----------------------*
set(kpilotinclude_HEADERS
	kpilotlink.h
	kpilotdevicelink.h
	kpilotlocallink.h
	pilot.h
	pilotDatabase.h
	pilotLinkVersion.h
	pilotLocalDatabase.h
	pilotRecord.h
	pilotSerialDatabase.h
	plugin.h
	pluginfactory.h
	syncAction.h
)

install(
	TARGETS kpilot
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(
	FILES ${kpilotinclude_HEADERS}
	DESTINATION ${CMAKE_INSTALL_PREFIX}/include/kpilot
)

install(
	FILES kpilotlib.kcfg DESTINATION ${KDE3_KCFG_DIR}
)

