### Makefile for the Expense Conduit
###
### The expense conduit is Copyright (C) 2001 by Chris Molnar

INCLUDES= $(all_includes) -I../../lib
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = expense-conduit.desktop

kde_module_LTLIBRARIES = libexpenseconduit.la


libexpenseconduit_la_SOURCES = expense-factory.cc setupDialog.cc expense.cc
libexpenseconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libexpenseconduit_la_LIBADD  =  -lkpilot

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = expense.h setupDialog.h 
