METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkdepim \
           -I$(top_srcdir)/libkpimidentities -I$(top_srcdir)/libkcal \
           -I$(top_srcdir)/kresources/imap/shared -I$(top_srcdir) \
           -I$(top_srcdir)/knotes \
           -I$(top_srcdir)/certmanager/lib \
           -I$(top_builddir)/kresources/groupwise \
           $(all_includes)

bin_PROGRAMS = groupwarewizard egroupwarewizard sloxwizard kolabwizard \
               groupwisewizard

kde_module_LTLIBRARIES = libegroupwarewizard.la libsloxwizard.la \
                         libkolabwizard.la libgroupwisewizard.la


groupwarewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
groupwarewizard_LDADD = libegroupwarewizard.la libsloxwizard.la $(LIB_KDEUI) \
                        libkolabwizard.la libgroupwisewizard.la
groupwarewizard_SOURCES = groupwarewizard.cpp main.cpp overviewpage.cpp

# Kolab
libkolabwizard_la_LDFLAGS = $(all_libraries) -no-undefined
libkolabwizard_la_LIBADD = $(top_builddir)/kresources/imap/kcal/libkcal_imap.la \
                           $(top_builddir)/kresources/imap/kabc/libkabc_imap.la \
                           $(top_builddir)/kresources/imap/knotes/libknotes_imap.la \
                           $(top_builddir)/libkcal/libkcal.la \
                           $(top_builddir)/libkdepim/libkdepim.la \
                           $(top_builddir)/libkpimidentities/libkpimidentities.la

libkolabwizard_la_SOURCES = kolabwizard.cpp kmailchanges.cpp kolabconfig.kcfgc \
  kolabkmailchanges.cpp

kolabwizard_LDADD = libkolabwizard.la $(LIB_KDEUI)
kolabwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kolabwizard_SOURCES = kolabmain.cpp

# eGroupware
libegroupwarewizard_la_LDFLAGS = $(all_libraries) -no-undefined
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
libsloxwizard_la_LDFLAGS = $(all_libraries) -no-undefined
libsloxwizard_la_SOURCES = sloxwizard.cpp sloxconfig.kcfgc
libsloxwizard_la_LIBADD = $(top_builddir)/kresources/slox/libkcal_slox.la \
                          $(top_builddir)/kresources/slox/libkabc_slox.la \
                          $(top_builddir)/libkdepim/libkdepim.la

sloxwizard_LDADD = libsloxwizard.la
sloxwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
sloxwizard_SOURCES = sloxmain.cpp

# GroupWise
libgroupwisewizard_la_LDFLAGS = $(all_libraries) -no-undefined
libgroupwisewizard_la_SOURCES = groupwisewizard.cpp groupwiseconfig.kcfgc \
  kmailchanges.cpp
libgroupwisewizard_la_LIBADD = $(top_builddir)/kresources/groupwise/libkcal_groupwise.la \
                          $(top_builddir)/kresources/groupwise/libkabc_groupwise.la \
                          $(top_builddir)/libkdepim/libkdepim.la \
                          $(top_builddir)/libkpimidentities/libkpimidentities.la
libgroupwisewizard_la_COMPILE_FIRST = $(top_builddir)/kresources/groupwise/kabc_groupwiseprefs.h \
                                      $(top_builddir)/kresources/groupwise/kcal_groupwiseprefsbase.h


groupwisewizard_LDADD = libgroupwisewizard.la
groupwisewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
groupwisewizard_SOURCES = groupwisemain.cpp

noinst_HEADERS = egroupwarewizard.h kmailchanges.h kolabwizard.h sloxwizard.h \
                 groupwisewizard.h

kde_kcfg_DATA = egroupware.kcfg slox.kcfg kolab.kcfg groupwise.kcfg

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kdepimwizards.pot
