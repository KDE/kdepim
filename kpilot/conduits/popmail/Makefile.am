### Makefile for the VCal and Todo conduits
###
### The conduits are copyrighted by all kinds of people, see
### the individual files.

INCLUDES= $(PISOCK_INCLUDE) -I$(top_srcdir)/kpilot/lib $(all_includes)
METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = popmail-conduit.desktop

####### This part is very kpilot specific
# you can add here more. This one gets installed 
kde_module_LTLIBRARIES = libpopmailconduit.la

# Which sources should be compiled for popmail_conduit
libpopmailconduit_la_SOURCES = popmail-factory.cc setupDialog.cc \
	passworddialog.cc popmail-conduit.cc parsedate.c
libpopmailconduit_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libpopmailconduit_la_LIBADD  =  ../../libkpilot.la

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = popmail-conduit.h passworddialog.h setupDialog.h

