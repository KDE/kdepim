## Makefile.am for rfc822 file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_rfc822.h

kde_module_LTLIBRARIES = kfile_rfc822.la

kfile_rfc822_la_SOURCES = kfile_rfc822.cpp 
kfile_rfc822_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_rfc822_la_LIBADD = $(LIB_KSYCOCA)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_rfc822.cpp -o $(podir)/kfile_rfc822.pot

services_DATA = kfile_rfc822.desktop
servicesdir = $(kde_servicesdir)
