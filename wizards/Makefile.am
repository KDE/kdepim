INCLUDES = -I$(top_srcdir)/libkdepim -I$(top_srcdir)/libkcal \
  -I$(top_srcdir)/kresources/imap/shared -I$(top_srcdir) \
  $(all_includes)

bin_PROGRAMS = kolabwizard

kolabwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kolabwizard_LDADD = ../kresources/imap/kcal/libkcal_imap.la \
  ../libkcal/libkcal.la ../libkdepim/libkdepim.la \
  $(LIB_KDECORE)
kolabwizard_SOURCES = kolabwizard.cpp kolabconfig.kcfgc

kde_kcfg_DATA = kolab.kcfg
