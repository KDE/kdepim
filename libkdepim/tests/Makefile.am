AM_CPPFLAGS = -I$(top_srcdir)/libkdepim $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

check_PROGRAMS = testwizard testaddresseelineedit testemail

testwizard_LDADD = ../libkdepim.la $(LIB_KDECORE)
testwizard_SOURCES = testwizard.cpp myconfig.kcfgc

testaddresseelineedit_LDADD = ../libkdepim.la $(LIB_KDECORE)
testaddresseelineedit_SOURCES = testaddresseelineedit.cpp

testemail_LDADD = ../libkdepim.la
testemail_SOURCES = testemail.cpp

METASOURCES = AUTO

#example_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#example_LDADD = ../libkdepim.la $(LIB_KDECORE)
#example_SOURCES = example.cpp exampleprefs_base.kcfgc

