SUBDIRS = egroupware

INCLUDES = -I$(top_srcdir)/libkdepim -I$(top_srcdir)/libkcal \
           -I$(top_srcdir)/kresources/imap/shared -I$(top_srcdir) \
           $(all_includes)

bin_PROGRAMS = groupwarewizard kolabwizard

groupwarewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
groupwarewizard_LDADD = $(LIB_KDEUI) \
                        $(top_builddir)/wizards/egroupware/libegroupwarewizard.la
groupwarewizard_SOURCES = actionpage.cpp groupwarewizard.cpp main.cpp \
                          overviewpage.cpp servertypemanager.cpp


kolabwizard_LDADD = $(top_srcdir)/kresources/imap/kcal/libkcal_imap.la \
                    $(top_srcdir)/libkcal/libkcal.la \
                    $(top_srcdir)/libkdepim/libkdepim.la \
                    $(LIB_KDECORE)
kolabwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kolabwizard_SOURCES = kolabmain.cpp kolabwizard.cpp kmailchanges.cpp kolabconfig.kcfgc

noinst_HEADERS = kmailchanges.h kolabwizard.h 

kde_kcfg_DATA = kolab.kcfg
