
SUBDIRS = ksieve . tests

INCLUDES = -I$(top_srcdir)/libkmime $(all_includes)

lib_LTLIBRARIES = libksieve.la

libksieve_la_SOURCES = error.cpp lexer.cpp parser.cpp utf8validator.cpp
libksieve_la_LIBADD = $(LIB_KDECORE)
libksieve_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -no-undefined

messages:
	$(XGETTEXT) *.cpp ksieve/*.h -o $(podir)/libksieve.pot
