###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir) \
	$(PISOCK_INCLUDE) $(all_includes)

METASOURCES = korganizerConduit.moc \
	todo-conduit.moc \
	todo-factory.moc \
	vcal-conduit.moc \
	vcal-conduitbase.moc \
	vcal-factory.moc \
	vcal-factorybase.moc

servicedir  = $(kde_servicesdir)
service_DATA = vcal-conduit.desktop todo-conduit.desktop

noinst_LTLIBRARIES = libvcalconduit_shared.la
kde_module_LTLIBRARIES = conduit_vcal.la conduit_todo.la


conduit_vcal_la_SOURCES = vcal-conduit.cc vcal-factory.cc vcal-setup.cc
conduit_vcal_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_vcal_la_LIBADD = ../../lib/libkpilot.la \
	../../../libkcal/libkcal.la \
	libvcalconduit_shared.la

conduit_todo_la_SOURCES = todo-factory.cc todo-setup.cc todo-conduit.cc
conduit_todo_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
conduit_todo_la_LIBADD = ../../lib/libkpilot.la \
	../../../libkcal/libkcal.la \
	libvcalconduit_shared.la

libvcalconduit_shared_la_SOURCES = korganizerConduit.ui \
	vcalconduitSettings.kcfgc vcal-factorybase.cc \
	vcal-setupbase.cc \
	vcal-conduitbase.cc

kde_kcfg_DATA = vcalconduitbase.kcfg

## Weird fake target
dummy.cc :
	echo "static int dummy_int = 0;" > dummy.cc

korganizerConduit.moc.lo : dummy.cc
	$(LIBTOOL) --silent --mode=compile $(CXX) -o korganizerConduit.moc.o -c dummy.cc

