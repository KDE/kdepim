###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir) \
	$(PISOCK_INCLUDE) $(all_includes)

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = vcal-conduit.desktop todo-conduit.desktop

noinst_LTLIBRARIES = libvcalconduit_shared.la
kde_module_LTLIBRARIES = conduit_vcal.la conduit_todo.la

libvcalconduit_shared_la_SOURCES = korganizerConduit.ui vcal-factorybase.cc \
	vcal-setup.cc vcal-conduitbase.cc

## Almost all source files for the vcal conduit are already in the shared
## library...
conduit_vcal_la_SOURCES = vcal-conduit.cc vcal-factory.cc
conduit_vcal_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_vcal_la_LIBADD = ../../lib/libkpilot.la ../../../libkcal/libkcal.la libvcalconduit_shared.la

conduit_todo_la_SOURCES = todo-factory.cc todo-setup.cc todo-conduit.cc
conduit_todo_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_todo_la_LIBADD = ../../lib/libkpilot.la ../../../libkcal/libkcal.la libvcalconduit_shared.la
