
INCLUDES = -I$(top_srcdir)/libkdenetwork \
        -I$(top_srcdir)/libkpgp \
	-I$(top_srcdir)/certmanager/lib \
	-I$(top_builddir)/libkdepim \
	$(GPGME_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkleopatra_ui.la

libkleopatra_ui_la_SOURCES = \
	progressbar.cpp \
	progressdialog.cpp \
	keylistview.cpp \
	keyselectiondialog.cpp \
	keyrequester.cpp \
	passphrasedialog.cpp \
	keyapprovaldialog.cpp \
	backendconfigwidget.cpp \
	dnattributeorderconfigwidget.cpp \
	cryptoconfigmodule.cpp \
	cryptoconfigdialog.cpp \
	directoryserviceswidgetbase.ui \
	directoryserviceswidget.cpp \
	adddirectoryservicedialog.ui \
	adddirectoryservicedialogimpl.cpp

kleodir = $(includedir)/kleo
kleo_HEADERS = \
	progressbar.h \
	progressdialog.h \
	keylistview.h \
	keyselectiondialog.h \
	keyrequester.h \
	passphrasedialog.h \
	keyapprovaldialog.h \
	backendconfigwidget.h \
	dnattributeorderconfigwidget.h

METASOURCES = AUTO
