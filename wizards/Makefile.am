INCLUDES = -I$(top_srcdir)/libkdepim $(all_includes)

bin_PROGRAMS = kolabwizard

kolabwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kolabwizard_LDADD = $(top_builddir)/libkdepim/libkdepim.la $(LIB_KDECORE)
kolabwizard_SOURCES = kolabwizard.cpp kolabconfig.kcfgc

kde_kcfg_DATA = kolab.kcfg
