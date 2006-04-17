#
# Macros for the 'kode' code generatation tools
#

# TODO: find kxml_compiler even if we are not building kdepim
set( KODE_XML_COMPILER_EXECUTABLE ${CMAKE_BINARY_DIR}/kode/kxml_compiler/kxml_compiler.sh )

MACRO (KODE_ADD_XML_PARSER _sources)
	foreach (_current_file ${ARGN})
		get_filename_component( _schema ${_current_file} ABSOLUTE )
		get_filename_component( _basename ${_schema} NAME_WE )

		set( _source_cpp ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp )
		set( _source_h   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h )
		set( _parser_cpp ${CMAKE_CURRENT_BINARY_DIR}/${_basename}_parser.cpp )
		set( _parser_h   ${CMAKE_CURRENT_BINARY_DIR}/${_basename}_parser.h )

		add_custom_command(
			OUTPUT ${_source_cpp} #${_source_h} ${_parser_cpp} ${_parser_h}
			COMMAND ${KODE_XML_COMPILER_EXECUTABLE}
			ARGS --external-parser ${_schema}
			MAIN_DEPENDENCY ${_schema}
			DEPENDS ${KODE_XML_COMPILER_EXECUATABLE}
		)
		# hack since the above OUTPUT line doesn't work with cmake 2.3.4-20060317
		add_custom_command( OUTPUT ${_source_h} DEPENDS ${_source_cpp} )
		add_custom_command( OUTPUT ${_parser_cpp} DEPENDS ${_source_cpp} )
		add_custom_command( OUTPUT ${_parser_h} DEPENDS ${_source_cpp} )

		set( ${_sources} ${${_sources}} ${_source_cpp} ${_parser_cpp} )
	endforeach (_current_file)
ENDMACRO (KODE_ADD_XML_PARSER)
