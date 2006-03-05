kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libksieve ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

## final breaks static use:
## If you feel like "fixing" it, better talk to mutz@kde.org first :)
#KDE_OPTIONS = nofinal
#
#INCLUDES = -I$(top_srcdir)/libksieve $(all_includes)
#
#noinst_LTLIBRARIES = libksieve_parser.la
#
#libksieve_parser_la_SOURCES = utf8validator.cpp lexer.cpp parser.cpp
#libksieve_parser_la_LIBADD = ../shared/libksieve_shared.la
#libksieve_parser_la_LDFLAGS = $(all_libraries) -no-undefined
#
