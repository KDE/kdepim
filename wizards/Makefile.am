METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkdepim -I$(top_srcdir)/libkcal \
           -I$(top_srcdir)/kresources/imap/shared -I$(top_srcdir) \
           -I$(top_srcdir)/knotes $(all_includes)

bin_PROGRAMS = groupwarewizard egroupwarewizard sloxwizard kolabwizard
kde_module_LTLIBRARIES = libegroupwarewizard.la libsloxwizard.la libkolabwizard.la


groupwarewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
groupwarewizard_LDADD = libegroupwarewizard.la libsloxwizard.la $(LIB_KDEUI) libkolabwizard.la
groupwarewizard_SOURCES = groupwarewizard.cpp main.cpp overviewpage.cpp

# Kolab
libkolabwizard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkolabwizard_la_LIBADD = $(top_builddir)/kresources/kolab/kcal/libkcalkolab.la \
                           $(top_builddir)/kresources/kolab/kabc/libkabckolab.la \
                           $(top_builddir)/kresources/kolab/knotes/libknoteskolab.la \
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
libsloxwizard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libsloxwizard_la_SOURCES = sloxwizard.cpp sloxconfig.kcfgc
libsloxwizard_la_LIBADD = $(top_builddir)/kresources/slox/libkcal_slox.la \
                          $(top_builddir)/kresources/slox/libkabc_slox.la \
                          $(top_builddir)/libkdepim/libkdepim.la

sloxwizard_LDADD = libsloxwizard.la
sloxwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
sloxwizard_SOURCES = sloxmain.cpp

noinst_HEADERS = egroupwarewizard.h kmailchanges.h kolabwizard.h sloxwizard.h

kde_kcfg_DATA = egroupware.kcfg slox.kcfg # kolab.kcfg

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kdepimwizards.pot
