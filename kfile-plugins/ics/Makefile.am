# Makefile.am for ics file meta info plugin

# set the include path for X, kdepim, qt and KDE
INCLUDES = \
  -I$(top_srcdir) \
  $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_ics.h

kde_module_LTLIBRARIES = kfile_ics.la

kfile_ics_la_SOURCES = kfile_ics.cpp
kfile_ics_la_LDFLAGS =  $(all_libraries) -module $(KDE_PLUGIN)
kfile_ics_la_LIBADD = $(top_builddir)/libkcal/libkcal.la $(LIB_KSYCOCA)


METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_ics.cpp -o $(podir)/kfile_ics.pot

services_DATA = kfile_ics.desktop
servicesdir = $(kde_servicesdir)
