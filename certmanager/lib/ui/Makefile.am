INCLUDES = -I$(top_srcdir)/libkdenetwork \
	-I$(top_srcdir)/certmanager/lib \
	$(GPGME_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libkleopatra_ui.la

libkleopatra_ui_la_SOURCES = \
	progressbar.cpp \
	progressdialog.cpp \
	keylistview.cpp

#libkleopatra_ui_la_LIBADD = $(LIB_KDEUI) ../kleo/libkleopatra_core.la
#libkleopatra_ui_la_LDFLAGS = $(all_libraries)

kleodir = $(includedir)/kleo
kleo_HEADERS = progressbar.h progressdialog.h keylistview.h

METASOURCES = AUTO
