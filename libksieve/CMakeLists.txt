include(CheckTypeSize)
check_type_size("unsigned long" SIZEOF_UNSIGNED_LONG)
configure_file(config-libksieve.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-libksieve.h )

include_directories( ${CMAKE_SOURCE_DIR}/libkdepim  )

set(libksieve_parser_SRCS 
	${CMAKE_SOURCE_DIR}/libksieve/parser/utf8validator.cpp 
	${CMAKE_SOURCE_DIR}/libksieve/parser/lexer.cpp 
	${CMAKE_SOURCE_DIR}/libksieve/parser/parser.cpp
	)

add_subdirectory( ksieve ) 
add_subdirectory( tests ) 



########### next target ###############

set(ksieve_LIB_SRCS
	${CMAKE_SOURCE_DIR}/libksieve/shared/error.cpp
	${libksieve_parser_SRCS})


kde4_add_library(ksieve SHARED ${ksieve_LIB_SRCS})

target_link_libraries(ksieve  ${KDE4_KDECORE_LIBS}  )

set_target_properties(ksieve PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS ksieve ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES libksieve_export.h  DESTINATION ${INCLUDE_INSTALL_DIR}/ksieve COMPONENT Devel)


########### install files ###############


