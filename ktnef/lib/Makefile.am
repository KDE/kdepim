INCLUDES = -I$(top_srcdir)/ktnef -I$(top_builddir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libktnef.la

libktnef_la_SOURCES = ktnefparser.cpp ktnefproperty.cpp ktnefattach.cpp mapi.cpp \
	ktnefpropertyset.cpp ktnefmessage.cpp ktnefwriter.cpp lzfu.cpp
libktnef_la_LIBADD = $(LIB_KDECORE) $(LIB_KIO)
libktnef_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -no-undefined
libktnef_la_METASOURCES = AUTO

noinst_HEADERS = mapi.h lzfu.h
