INCLUDES = -I$(top_srcdir)/libksieve $(all_includes)

noinst_LTLIBRARIES = libksieve_parser.la

libksieve_parser_la_SOURCES = lexer.cpp utf8validator.cpp parser.cpp
libksieve_parser_la_LIBADD = ../shared/libksieve_shared.la
libksieve_parser_la_LDFLAGS = $(all_libraries) -no-undefined

