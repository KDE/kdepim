AM_CPPFLAGS = -I$(top_srcdir)/libemailfunctions $(all_includes)
LDADD = ../libemailfunctions.la $(LIB_KDECORE)

check_PROGRAMS = testidmapper

testidmapper_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testidmapper_SOURCES = testidmapper.cpp

METASOURCES = AUTO
