METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/libkdepim -I$(top_srcdir)/libkcal \
           -I$(top_srcdir)/kresources/imap/shared -I$(top_srcdir) \
           $(all_includes)

bin_PROGRAMS = groupwarewizard kolabwizard egroupwarewizard

groupwarewizard_LDFLAGS = \
                          $(all_libraries) $(KDE_RPATH)
groupwarewizard_LDADD = ../kresources/imap/kcal/libkcal_imap.la libegroupwarewizard.la libkolabwizard.la $(LIB_KDEUI)
groupwarewizard_SOURCES = groupwarewizard.cpp main.cpp \
                          overviewpage.cpp

kde_module_LTLIBRARIES = libegroupwarewizard.la libkolabwizard.la

libkolabwizard_la_LDADD = ../kresources/imap/kcal/libkcal_imap.la \
                          ../libkcal/libkcal.la \
                          ../libkdepim/libkdepim.la \
                          $(LIB_KDECORE)
libkolabwizard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkolabwizard_la_SOURCES = kolabwizard.cpp kmailchanges.cpp kolabconfig.kcfgc

kolabwizard_LDADD = libkolabwizard.la  ../kresources/imap/kcal/libkcal_imap.la \
                    ../libkcal/libkcal.la \
                    ../libkdepim/libkdepim.la \
                    $(LIB_KDEUI)
kolabwizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kolabwizard_SOURCES = kolabmain.cpp


libegroupwarewizard_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libegroupwarewizard_la_LIBADD = $(top_builddir)/kresources/egroupware/libkabc_xmlrpc.la \
                                $(top_builddir)/kresources/egroupware/libkcal_xmlrpc.la \
                                $(top_builddir)/libkcal/libkcal.la \
                                $(top_builddir)/libkdepim/libkdepim.la \
                                $(LIB_KDEUI)
libegroupwarewizard_la_SOURCES = editdialog.cpp egroupwarehandler.cpp \
                                 egroupwarewizard.cpp actionpage.cpp \
                                 servertypemanager.cpp

egroupwarewizard_LDADD = libegroupwarewizard.la
egroupwarewizard_LDFLAGS = $(all_libraries) $(KDE_RPATH)
egroupwarewizard_SOURCES = egroupwaremain.cpp


noinst_HEADERS = kmailchanges.h kolabwizard.h

kde_kcfg_DATA = kolab.kcfg
