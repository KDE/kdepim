# final breaks anonymous namespace use:
KDE_OPTIONS = nofinal
SUBDIRS = ksieve shared parser . tests

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libksieve.la

CLEANFILES = dummy.cpp

libksieve_la_SOURCES = dummy.cpp
libksieve_la_LIBADD = \
	parser/libksieve_parser.la
libksieve_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -no-undefined

dummy.cpp:
	echo > dummy.cpp

messages:
	$(XGETTEXT) shared/*.cpp parser/*.cpp impl/*.h ksieve/*.h -o $(podir)/libksieve.pot
