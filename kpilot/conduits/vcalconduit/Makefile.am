###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir) \
	$(PISOCK_INCLUDE) $(all_includes)

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = vcal-conduit.desktop todo-conduit.desktop

kde_module_LTLIBRARIES = libvcalconduit.la libtodoconduit.la

libvcalconduit_la_SOURCES = korganizerConduit.ui vcal-factorybase.cc vcal-factory.cc  \
	vcal-setup.cc vcal-conduitbase.cc vcal-conduit.cc
libvcalconduit_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libvcalconduit_la_LIBADD = ../../lib/libkpilot.la ../../../libkcal/libkcal.la

libtodoconduit_la_SOURCES = korganizerConduit.ui vcal-factorybase.cc todo-factory.cc  \
	vcal-setup.cc todo-setup.cc vcal-conduit.cc vcal-conduitbase.cc todo-conduit.cc vcal-factory.cc 
libtodoconduit_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libtodoconduit_la_LIBADD = ../../lib/libkpilot.la ../../../libkcal/libkcal.la
