INCLUDES = -I$(top_srcdir)/libkdenetwork \
	-I$(top_srcdir)/certmanager/lib \
	$(GPGME_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkleopatra_ui.la

libkleopatra_ui_la_SOURCES = \
	progressbar.cpp \
	progressdialog.cpp \
	keylistview.cpp \
	keyselectiondialog.cpp

kleodir = $(includedir)/kleo
kleo_HEADERS = progressbar.h progressdialog.h keylistview.h keyselectiondialog.h

METASOURCES = AUTO
