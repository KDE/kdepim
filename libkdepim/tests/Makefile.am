AM_CPPFLAGS = -I$(top_srcdir)/libkdepim $(all_includes)

check_PROGRAMS = testwizard testaddresseelineedit testdistrlist

testwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testwizard_LDADD = ../libkdepim.la $(LIB_KDECORE)
testwizard_SOURCES = testwizard.cpp myconfig.kcfgc

testaddresseelineedit_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testaddresseelineedit_LDADD = ../libkdepim.la $(LIB_KDECORE)
testaddresseelineedit_SOURCES = testaddresseelineedit.cpp

testdistrlist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testdistrlist_LDADD = ../libkdepim.la
testdistrlist_SOURCES = testdistrlist.cpp

TESTS = testdistrlist

METASOURCES = AUTO

#example_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#example_LDADD = ../libkdepim.la $(LIB_KDECORE)
#example_SOURCES = example.cpp exampleprefs_base.kcfgc

