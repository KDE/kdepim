###
### Makefile for vcal and todo conduits. These two conduits share most code,
### so it seems logical to put them in one place.
###

INCLUDES= -I$(top_srcdir)/kpilot/lib -I$(top_srcdir)/libkcal \
	$(PISOCK_INCLUDE) $(all_includes)

METASOURCES = AUTO

servicedir  = $(kde_servicesdir)
service_DATA = vcal-conduit.desktop

kde_module_LTLIBRARIES = libvcalconduit.la

libvcalconduit_la_SOURCES = vcal-factory.cc korganizerConduit.ui \
	vcal-setup.cc vcal-conduit.cc
libvcalconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib -L../../../libkcal
libvcalconduit_la_LIBADD = -lkpilot -lkcal

