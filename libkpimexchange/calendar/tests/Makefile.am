INCLUDES = -I.. -I$(top_srcdir)/libkcal $(all_includes)

LDFLAGS = $(KDE_RPATH)

check_PROGRAMS = testdateset

METASOURCES = AUTO

testdateset_LDADD = ../../libkpimexchange.la 
testdateset_LDFLAGS = $(all_libraries)

testdateset_SOURCES = testdateset.cpp

