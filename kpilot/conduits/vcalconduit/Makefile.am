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
libvcalconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib -L../../../libkcal
libvcalconduit_la_LIBADD = -lkpilot -lkcal

libtodoconduit_la_SOURCES = korganizerConduit.ui vcal-factorybase.cc vcal-factory.cc todo-factory.cc  \
	vcal-setup.cc todo-setup.cc vcal-conduitbase.cc todo-conduit.cc
libtodoconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib -L../../../libkcal
libtodoconduit_la_LIBADD = -lkpilot -lkcal

# You may want to enable debugging like this on systems with broken kdelibs
# libtodoconduit_la_CFLAGS = -DDEBUG -DDEBUG_CERR
# libvcalconduit_la_CFLAGS = -DDEBUG -DDEBUG_CERR
#
# KDE_CXXFLAGS=-DDEBUG -DDEBUG_CERR
