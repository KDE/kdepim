#############################################################################

SUBDIRS = pics import lib ui
APPSDIR = $(kde_appsdir)/Internet
KDE_CXXFLAGS = $(USE_RTTI)
INCLUDES = -I$(srcdir)/lib -I$(srcdir)/ui -I$(top_srcdir)/rmm $(all_includes)

#############################################################################
# Compile as a library for kinit
#############################################################################

lib_LTLIBRARIES = libempath.la

libempath_la_SOURCES = main.cpp
libempath_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module
libempath_la_LIBADD = \
	$(top_builddir)/empath/lib/libempathkernel.la \
	$(top_builddir)/empath/ui/libempathui.la \
  $(top_builddir)/rmm/librmm.la

#############################################################################
# Compile as a binary
#############################################################################

bin_PROGRAMS = empath

empath_SOURCES = main.cpp
empath_LDFLAGS = $(KDE_RPATH) $(all_libraries)
empath_LDADD = \
	$(top_builddir)/empath/lib/libempathkernel.la \
	$(top_builddir)/empath/ui/libempathui.la \
  $(top_builddir)/rmm/librmm.la

#############################################################################
# i18n
#############################################################################

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot \
	-o $(podir)/empath.pot */*.cpp *.cpp

#############################################################################
# Extra data to install
#############################################################################

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) $(srcdir)/empath.desktop $(APPSDIR)
	$(mkinstalldirs) $(kde_datadir)/empath
	$(INSTALL_DATA) $(srcdir)/empath_logo.png $(kde_datadir)/empath/empath_logo.png

#############################################################################
# How to uninstall
#############################################################################
	
uninstall-local:
	-rm -f $(APPSDIR)/empath.desktop
	-rm -rf $(kde_datadir)/empath

#############################################################################
