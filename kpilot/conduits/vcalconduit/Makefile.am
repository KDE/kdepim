###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir)/libkcal \
	$(PISOCK_INCLUDE) $(all_includes)

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = vcal-conduit.desktop todo-conduit.desktop

kde_module_LTLIBRARIES = libvcalconduit.la libtodoconduit.la

libvcalconduit_la_SOURCES = vcal-factory.cc korganizerConduit.ui \
	vcal-setup.cc vcal-conduit.cc
libvcalconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib -L../../../libkcal
libvcalconduit_la_LIBADD = -lkpilot -lkcal

libtodoconduit_la_SOURCES = todo-factory.cc korganizertodoConduit.ui \
	todo-setup.cc todo-conduit.cc
libtodoconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib -L../../../libkcal
libtodoconduit_la_LIBADD = -lkpilot -lkcal

