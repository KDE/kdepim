# $Id$

SUBDIRS=core

METASOURCES = AUTO

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkpimexchange.la

libkpimexchange_la_SOURCES = dummy.cpp
libkpimexchange_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0
libkpimexchange_la_LIBADD = core/libkpimexchangecore.la \
			    $(LIB_KIO) $(top_builddir)/libkcal/libkcal.la 

dummy.cpp:
	echo >dummy.cpp

messages:
	$(XGETTEXT) core/*.cpp -o $(podir)/libkpimexchange.pot

include $(top_srcdir)/doc/api/Doxyfile.am
