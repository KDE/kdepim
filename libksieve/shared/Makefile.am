# final breaks anonymous namespaces use:
KDE_OPTIONS = nofinal

INCLUDES = -I$(top_srcdir)/libksieve $(all_includes)

noinst_LTLIBRARIES = libksieve_shared.la

libksieve_shared_la_SOURCES = error.cpp
libksieve_shared_la_LIBADD = $(LIB_KDECORE)
libksieve_shared_la_LDFLAGS = $(all_libraries) -no-undefined

