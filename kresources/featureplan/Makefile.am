INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkcal_resourcefeatureplan.la

noinst_LTLIBRARIES = libfeatureplancommon.la
libfeatureplancommon_la_SOURCES = kde-features.cpp kde-features_parser.cpp \
                                  prefs.kcfgc

libkcal_resourcefeatureplan_la_SOURCES = kcal_resourcefeatureplan.cpp \
                                         kcal_resourcefeatureplanconfig.cpp 
libkcal_resourcefeatureplan_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                                         -version-info 1:0:0 -no-undefined
libkcal_resourcefeatureplan_la_LIBADD = libfeatureplancommon.la \
                                        $(top_builddir)/libkcal/libkcal.la \
                                        $(top_builddir)/libkdepim/libkdepim.la
libkcal_resourcefeatureplan_la_COMPILE_FIRST = kde-features.h prefs.h


kde_module_LTLIBRARIES = kcal_resourcefeatureplan.la

kcal_resourcefeatureplan_la_SOURCES = kcal_resourcefeatureplanplugin.cpp 
kcal_resourcefeatureplan_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_resourcefeatureplan_la_LIBADD = libkcal_resourcefeatureplan.la
kcal_resourcefeatureplan_la_COMPILE_FIRST = kde-features.h prefs.h

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_resourcefeatureplan.desktop

check_PROGRAMS = dumpfeaturelist benchmarkfeaturelist

dumpfeaturelist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
dumpfeaturelist_LDADD   = libfeatureplancommon.la $(LIB_KDECORE)
dumpfeaturelist_SOURCES = dumpfeaturelist.cpp
dumpfeatuelist_COMPILE_FIRST = kde-features.h

benchmarkfeaturelist_LDFLAGS = $(all_libraries) $(KDE_RPATH)
benchmarkfeaturelist_LDADD   = libfeatureplancommon.la $(LIB_KDECORE)
benchmarkfeaturelist_SOURCES = benchmarkfeaturelist.cpp
benchmarkfeatuelist_COMPILE_FIRST = kde-features.h

kde-features.h kde-features.cpp kde-features_parser.h kde-features_parser.cpp: $(srcdir)/kde-features.rng \
  $(top_builddir)/kode/kxml_compiler/kxml_compiler
	$(top_builddir)/kode/kxml_compiler/kxml_compiler \
          --custom-parser $(srcdir)/kde-features.rng

METASOURCES = AUTO
DISTCLEANFILES = kde-features.h kde-features.cpp kde-features_parser.h kde-features_parser.cpp
