### Makefile for KPilot's doc  conduit
###
### The doc conduit is Copyright (C) 2002 by Reinhold Kainhofer
### the files makedoc9.{h,cpp} are also Copyright (C) 2000 by Pat Beirne



INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = doc_conduit.desktop

kde_module_LTLIBRARIES = libdocconduit.la
libdocconduit_la_SOURCES = makedoc9.cc pilotDOCHead.cc pilotDOCEntry.cc pilotDOCBookmark.cc doc-factory.cc doc-setup.cc doc-conduit.cc DOC-converter.cc doc-setupdialog.ui doc-conflictdialog.cc
libdocconduit_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libdocconduit_la_LIBADD = ../../lib/libkpilot.la

SUBDIRS = Icons
bin_PROGRAMS = kpalmdoc

kpalmdoc_SOURCES = converterdlg_base.ui kpalmdoc.cpp
kpalmdoc_LDFLAGS = $(all_libraries) $(KDE_RPATH) -L../../lib
kpalmdoc_LDADD = $(LIB_KFILE) $(PISOCK_LIB) ../../lib/libkpilot.la 

appsdir = $(kde_appsdir)/Utilities
servicetypedir = $(kde_servicetypesdir)
apps_DATA = kpalmdoc.desktop 

#rcdir = $(kde_datadir)/kpalmdoc
#rc_DATA = kpilotui.rc

