#############################################################################

SUBDIRS = pics import lib parts ui
APPSDIR = $(kde_appsdir)/Internet
KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS) -DQT_NO_ASCII_CAST
INCLUDES = -I$(srcdir)/lib -I$(srcdir)/ui -I$(top_srcdir)/librmm $(all_includes)

#############################################################################
# Compile as a library for kinit
#############################################################################

lib_LTLIBRARIES = empath.la

empath_la_SOURCES = main.cpp

empath_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module

empath_la_LIBADD = \
	$(top_builddir)/empath/lib/libempathkernel.la \
	$(top_builddir)/empath/ui/libempathui.la \
	$(top_builddir)/librmm/librmm.la

#############################################################################
# Compile as a binary
#############################################################################

bin_PROGRAMS = empath

empath_SOURCES  = dummy.cpp
empath_LDFLAGS  = $(all_libraries)
empath_LDADD    = empath.la

dummy.cpp:
	echo > dummy.cpp

#############################################################################
# i18n
#############################################################################

messages:
	$(XGETTEXT) -C -kI18N_NOOP -ki18n -x $(includedir)/kde.pot \
	-o $(podir)/empath.pot */*.cpp *.cpp

#############################################################################
# Extra data to install
#############################################################################

data_DATA = empath.desktop
datadir = $(kde_appsdir)
EXTRA_DIST = $(data_DATA)

install-data-local: 
	$(mkinstalldirs) $(kde_datadir)/empath
	$(INSTALL_DATA) $(srcdir)/empath_logo.png $(kde_datadir)/empath/empath_logo.png

#############################################################################
# How to uninstall
#############################################################################
	
uninstall-local:
	-rm -rf $(kde_datadir)/empath

#############################################################################
