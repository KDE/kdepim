#############################################################################

SUBDIRS = pics lib parts ui
appsdir = $(kde_appsdir)/Internet
apps_DATA = twister.desktop
KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS) -DQT_NO_ASCII_CAST
INCLUDES = -I$(srcdir)/lib -I$(srcdir)/ui $(all_includes)

#############################################################################
# Compile as a library for kinit
#############################################################################

lib_LTLIBRARIES = twister.la

twister_la_SOURCES = main.cpp
twister_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module
# Define this for testing.
# -no-undefined

twister_la_LIBADD = lib/libtwisterkernel.la ui/libtwisterui.la

#############################################################################
# Compile as a binary
#############################################################################

bin_PROGRAMS = twister

twister_SOURCES = dummy.cpp
twister_LDFLAGS = $(all_libraries)
twister_LDADD = lib/libtwisterkernel.la ui/libtwisterui.la

dummy.cpp:
	echo > $@

datadir = $(kde_datadir)/twister
# data_DATA = twister_logo.png

#############################################################################
# i18n
#############################################################################

messages:
	$(XGETTEXT) */*.cpp *.cpp -o $(podir)/twister.pot

