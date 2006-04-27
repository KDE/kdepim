#
# Find the Kode tools
#
# The following variables are defined by this module:
# 
# KODE_XML_COMPILER_EXECUTABLE - the kxml_compiler executable
#
# The following macros are provieded (from KodeMacros.cmake):
#
# kode_add_xml_parser (name schema1 ... schemaN)
#   Generate code to handle XML defined by the given schemata
#


# check if we are currently building kdepim
if ( EXISTS ${CMAKE_SOURCE_DIR}/libkdepim/kdepimmacros.h )
	set( KODE_XML_COMPILER_EXECUTABLE ${CMAKE_BINARY_DIR}/kode/kxml_compiler/kxml_compiler.sh )
	set( _KODE_XML_COMPILER_DEP kxml_compiler )
else ( EXISTS ${CMAKE_SOURCE_DIR}/libkdepim/kdepimmacros.h )
	find_program( KODE_XML_COMPILER_EXECUTABLE NAME kxml_compiler PATHS
		${KDE4_BIN_INSTALL_DIR}
		$ENV{KDEDIR}/bin
		/opt/kde/bin
		/opt/kde4/bin
		NO_SYSTEM_PATH
		NO_CMAKE_SYSTEM_PATH
	)

	if ( NOT KODE_XML_COMPILER_EXECUTABLE )
		find_program( KODE_XML_COMPILER_EXECUTABLE NAME kxml_compiler )
	endif ( NOT KODE_XML_COMPILER_EXECUTABLE )

	if ( KODE_XML_COMPILER_EXECUTABLE )
		message( STATUS "Found Kode XML compiler: ${KODE_XML_COMPILER_EXECUTABLE}" )
	else ( KODE_XML_COMPILER_EXECUTABLE )
		message( FATAL_ERROR "Couldn't find kxml_compiler" )
	endif ( KODE_XML_COMPILER_EXECUTABLE )

	set( _KODE_XML_COMPILER_DEP )
endif ( EXISTS ${CMAKE_SOURCE_DIR}/libkdepim/kdepimmacros.h )

# macros to use the Kode tools
include( KodeMacros )
