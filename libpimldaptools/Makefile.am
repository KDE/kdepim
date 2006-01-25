INCLUDES = $(all_includes)

lib_LTLIBRARIES = libpimldaptools.la
libpimldaptools_la_SOURCES = \
			       ldapconfigwidget.cpp \
			       ldif.cpp \
			       ldapurl.cpp
libpimldaptools_la_LIBADD = $(LIB_KDECORE)  $(LIB_KIO)
libpimldaptools_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0

METASOURCES = AUTO

# No messages target. Those files are part of libkdepim.pot.
