INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libemailfunctions.la
libemailfunctions_la_SOURCES = email.cpp idmapper.cpp kasciistricmp.cpp \
                               kasciistringtools.cpp \
                               networkstatus.cpp networkstatus.skel
libemailfunctions_la_LDFLAGS = $(all_libraries) -no-undefined
## Since this is a noinst library, in case of --enable-closure we need
## to link it explicitly to the libraries that it uses.
libemailfunctions_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

emailfunctionsincludedir = $(includedir)/libemailfunctions
emailfunctionsinclude_HEADERS = idmapper.h

METASOURCES = AUTO

# No messages target. Those files are part of libkdepim.pot.

include $(top_srcdir)/doc/api/Doxyfile.am
