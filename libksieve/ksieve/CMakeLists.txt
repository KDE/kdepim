kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############

install_files( /include/ksieve FILES  )

kde4_footer()



#original Makefile.am contents follow:

## here are header files that are part of the public api:
#ksievedir = $(includedir)/ksieve
#
#ksieve_HEADERS = \
#	error.h \
#	lexer.h \
#	parser.h \
#	scriptbuilder.h
