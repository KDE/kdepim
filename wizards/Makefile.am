METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkdepim -I$(top_srcdir)/libkcal \
           -I$(top_srcdir)/kresources/imap/shared -I$(top_srcdir) \
           $(all_includes)

bin_PROGRAMS = groupwarewizard kolabwizard egroupwarewizard sloxwizard
kde_module_LTLIBRARIES = libegroupwarewizard.la libkolabwizard.la


groupwarewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
groupwarewizard_LDADD = libegroupwarewizard.la libkolabwizard.la $(LIB_KDEUI)
groupwarewizard_SOURCES = groupwarewizard.cpp main.cpp overviewpage.cpp

# Kolab
libkolabwizard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkolabwizard_la_LIBADD = $(top_builddir)/kresources/imap/kcal/libkcal_imap.la \
                           $(top_builddir)/libkcal/libkcal.la \
                           $(top_builddir)/libkdepim/libkdepim.la
libkolabwizard_la_SOURCES = kolabwizard.cpp kmailchanges.cpp kolabconfig.kcfgc

kolabwizard_LDADD = libkolabwizard.la $(LIB_KDEUI)
kolabwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kolabwizard_SOURCES = kolabmain.cpp

# eGroupware
libegroupwarewizard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libegroupwarewizard_la_SOURCES = egroupwarewizard.cpp egroupwareconfig.kcfgc
libegroupwarewizard_la_LIBADD = $(top_builddir)/kresources/egroupware/libkabc_xmlrpc.la \
                                $(top_builddir)/kresources/egroupware/libkcal_xmlrpc.la \
                                $(top_builddir)/kresources/egroupware/libknotes_xmlrpc.la \
                                $(top_builddir)/libkcal/libkcal.la \
                                $(top_builddir)/libkdepim/libkdepim.la \
                                $(LIB_KDEUI)

egroupwarewizard_LDADD = libegroupwarewizard.la
egroupwarewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
egroupwarewizard_SOURCES = egroupwaremain.cpp

# SLOX
sloxwizard_LDADD = ../kresources/slox/libkcal_slox.la \
                   ../kresources/slox/libkabc_slox.la \
                   ../libkdepim/libkdepim.la
sloxwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
sloxwizard_SOURCES = sloxmain.cpp sloxwizard.cpp sloxconfig.kcfgc


noinst_HEADERS = kmailchanges.h kolabwizard.h egroupwarewizard.h

kde_kcfg_DATA = egroupware.kcfg kolab.kcfg slox.kcfg
