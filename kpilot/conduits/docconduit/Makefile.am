### Makefile for KPilot's doc  conduit
###
### The doc conduit is Copyright (C) 2002 by Reinhold Kainhofer
### the files makedoc9.{h,cpp} are also Copyright (C) 2000 by Pat Beirne

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = doc_conduit.desktop

kde_module_LTLIBRARIES = libdocconduit.la

libdocconduit_la_SOURCES = makedoc9.cc pilotDOCHead.cc pilotDOCEntry.cc pilotDOCBookmark.cc doc-factory.cc doc-setup.cc doc-conduit.cc DOC-converter.cc doc-setupdialog.ui
libdocconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib 
libdocconduit_la_LIBADD = -lkpilot

