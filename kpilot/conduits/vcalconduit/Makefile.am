###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= $(all_includes) -I$(top_srcdir)/kpilot/lib

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = todo-conduit.desktop vcal-conduit.desktop

kde_module_LTLIBRARIES = libtodoconduit.la libvcalconduit.la

libtodoconduit_la_SOURCES = todo-factory.cc korganizertodoConduit.ui
libtodoconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libtodoconduit_la_LIBADD = -lkpilot

libvcalconduit_la_SOURCES = vcal-factory.cc korganizerConduit.ui
libvcalconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libvcalconduit_la_LIBADD = -lkpilot

