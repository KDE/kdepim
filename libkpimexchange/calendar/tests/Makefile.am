INCLUDES = -I.. -I$(top_srcdir)/libkcal $(all_includes)

check_PROGRAMS = testdateset

METASOURCES = AUTO

testdateset_LDADD = ../../libkpimexchange.la 
testdateset_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testdateset_SOURCES = testdateset.cpp
