AM_CPPFLAGS = -I$(top_srcdir)/libemailfunctions $(all_includes)
LDADD = ../libemailfunctions.la $(LIB_KDECORE)

check_PROGRAMS = testidmapper testemail
TESTS = testemail

testidmapper_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testidmapper_SOURCES = testidmapper.cpp

testemail_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testemail_SOURCES = testemail.cpp

METASOURCES = AUTO
