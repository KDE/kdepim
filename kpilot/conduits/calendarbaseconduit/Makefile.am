###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir) \
	$(PISOCK_INCLUDE) $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libconduit_vcalbase.la

libconduit_vcalbase_la_SOURCES = vcalconduitSettings.kcfgc \
	korganizerConduit.ui \
	vcal-factorybase.cc \
	vcal-setupbase.cc \
	vcal-conduitbase.cc
libconduit_vcalbase_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libconduit_vcalbase_la_LIBADD = ../../lib/libkpilot.la \
	../../../libkcal/libkcal.la
	
kpilotincludedir = $(includedir)/kpilot
kpilotinclude_HEADERS = vcalconduitSettings.h korganizerConduit.h vcal-factorybase.h vcal-setupbase.h vcal-conduitbase.h

kde_kcfg_DATA = vcalconduitbase.kcfg

