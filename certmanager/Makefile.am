SUBDIRS = lib conf .

KDE_CXXFLAGS = -DLIBKLEOPATRA_NO_COMPAT

INCLUDES = -I$(srcdir)/lib -I$(top_srcdir)/libkdenetwork $(GPGME_CFLAGS) $(all_includes)

bin_PROGRAMS = kgpgcertmanager

kgpgcertmanager_SOURCES = \
	customactions.cpp \
	aboutdata.cpp \
	main.cpp certmanager.cpp \
	certificatewizard.ui certificatewizardimpl.cpp \
	certificateinfowidget.ui certificateinfowidgetimpl.cpp \
	crlview.cpp storedtransferjob.cpp

METASOURCES = AUTO

kgpgcertmanager_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kgpgcertmanager_LDADD   = conf/libconf.la lib/libkleopatra.la $(LIB_KUTILS) $(LIB_KABC)

rcdir = $(kde_datadir)/kgpgcertmanager
rc_DATA = kgpgcertmanagerui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kgpgcertmanager.pot

include ../admin/Doxyfile.am
