# final breaks static use:
# If you feel like "fixing" it, better talk to mutz@kde.org first :)
KDE_OPTIONS = nofinal

INCLUDES = -I$(top_srcdir)/libksieve $(all_includes)

noinst_LTLIBRARIES = libksieve_parser.la

libksieve_parser_la_SOURCES = utf8validator.cpp lexer.cpp parser.cpp
libksieve_parser_la_LIBADD = ../shared/libksieve_shared.la
libksieve_parser_la_LDFLAGS = $(all_libraries) -no-undefined

