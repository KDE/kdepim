### Makefile for the Expense Conduit
###
### The expense conduit is Copyright (C) 2001 by Chris Molnar

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = expense-conduit.desktop

kde_module_LTLIBRARIES = libexpenseconduit.la


libexpenseconduit_la_SOURCES = expenseConduit.ui setupDialog.cc \
	expense.cc expense-factory.cc
libexpenseconduit_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libexpenseconduit_la_LIBADD  = ../../lib/libkpilot.la $(LIB_KIO)

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = expense.h setupDialog.h 
