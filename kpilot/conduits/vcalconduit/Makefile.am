###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir) \
	$(PISOCK_INCLUDE) $(all_includes)

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = vcal-conduit.desktop todo-conduit.desktop

kde_module_LTLIBRARIES = conduit_vcal.la conduit_todo.la

conduit_vcal_la_SOURCES = korganizerConduit.ui vcal-factorybase.cc vcal-factory.cc  \
	vcal-setup.cc vcal-conduitbase.cc vcal-conduit.cc
conduit_vcal_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_vcal_la_LIBADD = ../../lib/libkpilot.la ../../../libkcal/libkcal.la

conduit_todo_la_SOURCES = korganizerConduit.ui vcal-factorybase.cc todo-factory.cc  \
	vcal-setup.cc todo-setup.cc vcal-conduit.cc vcal-conduitbase.cc todo-conduit.cc vcal-factory.cc 
conduit_todo_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_todo_la_LIBADD = ../../lib/libkpilot.la ../../../libkcal/libkcal.la
