### Makefile for KPilot's doc  conduit
###
### The doc conduit is Copyright (C) 2002 by Reinhold Kainhofer
### the files makedoc9.{h,cpp} are also Copyright (C) 2000 by Pat Beirne

SUBDIRS = Icons

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO


servicedir = $(kde_servicesdir)
service_DATA = doc_conduit.desktop

noinst_LTLIBRARIES = libpalmdoc_shared.la
kde_module_LTLIBRARIES = conduit_doc.la
bin_PROGRAMS = kpalmdoc

libpalmdoc_shared_la_SOURCES = makedoc9.cc pilotDOCHead.cc pilotDOCEntry.cc pilotDOCBookmark.cc DOC-converter.cc

conduit_doc_la_SOURCES = doc-factory.cc doc-setup.cc doc-conduit.cc doc-setupdialog.ui doc-conflictdialog.cc 
conduit_doc_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_doc_la_LIBADD = ../../lib/libkpilot.la libpalmdoc_shared.la \
	$(LIB_KDEUI) $(LIB_KFILE)

##libpalmdoc_shared.la

kpalmdoc_SOURCES = kpalmdoc_dlgbase.ui kpalmdoc_dlg.cc kpalmdoc.cpp
kpalmdoc_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kpalmdoc_LDADD = $(LIB_KFILE) $(PISOCK_LIB) ../../lib/libkpilot.la libpalmdoc_shared.la

appsdir = $(kde_appsdir)/Utilities
servicetypedir = $(kde_servicetypesdir)
apps_DATA = kpalmdoc.desktop 
