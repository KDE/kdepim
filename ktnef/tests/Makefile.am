AM_CPPFLAGS = -I$(top_srcdir)/ktnef $(all_includes) -DKDESRCDIR=\"$(srcdir)\"
LDADD = ../lib/libktnef.la $(LIB_KDECORE)

check_PROGRAMS = parsertest
TESTS = parsertest

parsertest_LDFLAGS = $(all_libraries) $(KDE_RPATH)
parsertest_SOURCES = parsertest.cpp

METASOURCES = AUTO
