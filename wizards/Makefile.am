METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkpimidentities -I$(top_srcdir)/libkcal \
           -I$(top_srcdir)/kresources/kolab/shared -I$(top_srcdir) \
           -I$(top_srcdir)/knotes \
           -I$(top_srcdir)/certmanager/lib \
            -I$(top_builddir)/kresources/groupwise \
           -I$(top_builddir)/kresources/lib \
           -I$(top_srcdir)/kresources/lib \
           $(all_includes)

if include_exchange_SUBDIR
EXCHANGE_BIN_WIZARD=exchangewizard
EXCHANGE_LIB_LA=libexchangewizard.la
endif

if include_egroupware_SUBDIR
EGROUPWARE_BIN_WIZARD=egroupwarewizard
EGROUPWARE_LIB_LA=libegroupwarewizard.la
endif

if include_slox_SUBDIR
SLOX_BIN_WIZARD=sloxwizard
SLOX_LIB_LA=libsloxwizard.la
endif

if include_kolab_SUBDIR
KOLAB_BIN_WIZARD=kolabwizard
KOLAB_LIB_LA=libkolabwizard.la
endif

if include_groupwise_SUBDIR
GROUPWISE_BIN_WIZARD=groupwisewizard
GROUPWISE_LIB_LA=libgroupwisewizard.la
endif

if include_scalix_SUBDIR
SCALIX_BIN_WIZARD=scalixwizard
SCALIX_LIB_LA=libscalixwizard.la
endif

bin_PROGRAMS = groupwarewizard $(EGROUPWARE_BIN_WIZARD) $(SLOX_BIN_WIZARD) $(KOLAB_BIN_WIZARD) \
               $(GROUPWISE_BIN_WIZARD) $(EXCHANGE_BIN_WIZARD) $(SCALIX_BIN_WIZARD)

kde_module_LTLIBRARIES = $(EGROUPWARE_LIB_LA) $(SLOX_LIB_LA) \
                         $(KOLAB_LIB_LA) \
                         $(EXCHANGE_LIB_LA) \
                         $(GROUPWISE_LIB_LA) \
                         $(SCALIX_LIB_LA)


groupwarewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
groupwarewizard_LDADD = libegroupwarewizard.la libsloxwizard.la $(LIB_KDEUI) \
                        libkolabwizard.la \
                        libexchangewizard.la \
                        libgroupwisewizard.la
groupwarewizard_SOURCES = groupwarewizard.cpp main.cpp overviewpage.cpp

# Kolab
if include_kolab_SUBDIR
libkolabwizard_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
libkolabwizard_la_LIBADD = $(top_builddir)/kresources/kolab/kcal/libkcalkolab.la \
                           $(top_builddir)/kresources/kolab/kabc/libkabckolab.la \
                           $(top_builddir)/kresources/kolab/knotes/libknoteskolab.la \
                           $(top_builddir)/libkcal/libkcal.la \
                           $(top_builddir)/libkdepim/libkdepim.la \
                           $(top_builddir)/libkpimidentities/libkpimidentities.la

libkolabwizard_la_SOURCES = kolabwizard.cpp kmailchanges.cpp kolabconfig.kcfgc \
  kolabkmailchanges.cpp

kolabwizard_LDADD = libkolabwizard.la $(LIB_KDEUI)
kolabwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kolabwizard_SOURCES = kolabmain.cpp
endif

# eGroupware
if include_egroupware_SUBDIR
libegroupwarewizard_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
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
if include_slox_SUBDIR
libsloxwizard_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
libsloxwizard_la_SOURCES = sloxwizard.cpp sloxconfig.kcfgc
libsloxwizard_la_LIBADD = $(top_builddir)/kresources/slox/libkcal_slox.la \
                          $(top_builddir)/kresources/slox/libkabc_slox.la \
                          $(top_builddir)/libkdepim/libkdepim.la
endif

sloxwizard_LDADD = libsloxwizard.la
sloxwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
sloxwizard_SOURCES = sloxmain.cpp
endif

# GroupWise
if include_groupwise_SUBDIR
libgroupwisewizard_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
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
endif

# Exchange
if include_exchange_SUBDIR
libexchangewizard_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
libexchangewizard_la_SOURCES = exchangewizard.cpp
libexchangewizard_la_LIBADD = $(top_builddir)/kresources/newexchange/libkcal_newexchange.la \
                              $(top_builddir)/kresources/newexchange/libkabc_newexchange.la \
                              $(top_builddir)/libkdepim/libkdepim.la

exchangewizard_LDADD = libexchangewizard.la
exchangewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
exchangewizard_SOURCES = exchangemain.cpp
endif

# Scalix
if include_scalix_SUBDIR
libscalixwizard_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
libscalixwizard_la_LIBADD = $(top_builddir)/kresources/scalix/kcal/libkcalscalix.la \
                           $(top_builddir)/kresources/scalix/kabc/libkabcscalix.la \
                           $(top_builddir)/libkcal/libkcal.la \
                           $(top_builddir)/libkdepim/libkdepim.la \
                           $(top_builddir)/libkpimidentities/libkpimidentities.la

libscalixwizard_la_SOURCES = scalixwizard.cpp kmailchanges.cpp scalixconfig.kcfgc \
  scalixkmailchanges.cpp

scalixwizard_LDADD = libscalixwizard.la $(LIB_KDEUI)
scalixwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
scalixwizard_SOURCES = scalixmain.cpp
endif

noinst_HEADERS = egroupwarewizard.h kmailchanges.h kolabwizard.h sloxwizard.h \
                 groupwisewizard.h exchangewizard.h

kde_kcfg_DATA = egroupware.kcfg slox.kcfg kolab.kcfg groupwise.kcfg scalix.kcfg

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kdepimwizards.pot

if include_egroupware_SUBDIR
egroupwarewizard.lo: ../kresources/egroupware/kabc_egroupwareprefs.h ../kresources/egroupware/kcal_egroupwareprefs.h ../kresources/egroupware/knotes_egroupwareprefs.h
endif

if include_exchange_SUBDIR
exchangewizard.lo: ../kresources/lib/kresources_groupwareprefs.h
endif

if include_slox_SUBDIR
sloxwizard.lo: ../kresources/slox/kabcsloxprefs.h ../kresources/slox/kcalsloxprefs.h
endif

xdg_apps_DATA = groupwarewizard.desktop
