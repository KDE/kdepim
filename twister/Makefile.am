#############################################################################

SUBDIRS = pics lib parts ui
APPSDIR = $(kde_appsdir)/Internet
KDE_CXXFLAGS = -DQT_NO_ASCII_CAST
INCLUDES = \
  $(all_includes) \
  -I$(top_srcdir)/twister/lib \
  -I$(top_srcdir)/twister/ui

#############################################################################
# Compile as a library for kinit
#############################################################################

lib_LTLIBRARIES = twister.la

twister_la_SOURCES = main.cpp

twister_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module
# Define this for testing.
# -no-undefined

twister_la_LIBADD = \
	$(top_builddir)/twister/lib/libtwisterkernel.la \
	$(top_builddir)/twister/ui/libtwisterui.la

#############################################################################
# Compile as a binary
#############################################################################

bin_PROGRAMS = twister

twister_SOURCES = main.cpp

twister_LDFLAGS = $(all_libraries)

twister_LDADD = \
	$(top_builddir)/twister/lib/libtwisterkernel.la \
	$(top_builddir)/twister/ui/libtwisterui.la

#############################################################################
# i18n
#############################################################################

messages:
	$(XGETTEXT) */*.cpp *.cpp -o $(podir)/twister.pot

#############################################################################
# Extra data to install
#############################################################################

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) $(srcdir)/twister.desktop $(APPSDIR)
	$(mkinstalldirs) $(kde_datadir)/twister
#	$(INSTALL_DATA) $(srcdir)/twister_logo.png $(kde_datadir)/twister/twister_logo.png

#############################################################################
# How to uninstall
#############################################################################
	
uninstall-local:
	-rm -f $(APPSDIR)/twister.desktop
	-rm -rf $(kde_datadir)/twister

#############################################################################
