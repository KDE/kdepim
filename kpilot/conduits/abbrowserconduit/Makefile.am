INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH) -I$(top_srcdir)/kpilot/lib 

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = abbrowser_conduit.desktop

kde_module_LTLIBRARIES = libabbrowserconduit.la
noinst_PROGRAMS = testkab testdcop


libabbrowserconduit_la_SOURCES = \
	abbrowser-factory.cc \
	abbrowser-setup.cc \
	abbrowser-conduit.cc \
	abbrowserConduitConfig_base.ui \
	deconflictDialog_base.ui


libabbrowserconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libabbrowserconduit_la_LIBADD = ../../lib/libkpilot.la
libabbrowserconduit_la_CFLAGS = -DDEBUG -DDEBUG_CERR

testkab_SOURCES = testkabapi.cpp
testkab_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testkab_LDADD = $(LIB_KDECORE) -lkab

testdcop_SOURCES = testAbbrowserDCOP.cc contactentry.cpp
testdcop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testdcop_LDADD = $(LIB_KDECORE) $(LIB_KFILE)


