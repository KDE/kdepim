SUBDIRS = lib .

KDE_CXXFLAGS = -DLIBKLEOPATRA_NO_COMPAT

INCLUDES = -I$(srcdir)/lib -I$(top_srcdir)/libkdenetwork $(GPGME_CFLAGS) $(all_includes)

bin_PROGRAMS = kgpgcertmanager

kgpgcertmanager_SOURCES = \
	customactions.cpp \
	aboutdata.cpp \
	main.cpp certmanager.cpp \
	certificatewizard.ui certificatewizardimpl.cpp \
	certificateinfowidget.ui certificateinfowidgetimpl.cpp \
	crlview.cpp

METASOURCES = AUTO

AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD    = $(LIB_KDEUI) $(LIB_KFILE) $(LIB_KABC) lib/libkleopatra.la

rcdir = $(kde_datadir)/kgpgcertmanager
rc_DATA = kgpgcertmanagerui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kgpgcertmanager.pot

include ../admin/Doxyfile.am
