INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libemailfunctions.la
libemailfunctions_la_SOURCES = email.cpp idmapper.cpp
libemailfunctions_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0:0
## Since this is a noinst library, in case of --enable-closure we need
## to link it explicitly to the libraries that it uses.
libemailfunctions_la_LIBADD = $(LIB_QT) $(LIB_KDECORE)

METASOURCES = AUTO

# No messages target. Those files are part of libkdepim.pot.

