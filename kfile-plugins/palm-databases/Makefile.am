## Makefile.am for palm database meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(PISOCK_INCLUDE) $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_palm.h

kde_module_LTLIBRARIES = kfile_palm.la

kfile_palm_la_SOURCES = kfile_palm.cpp 
kfile_palm_la_LDFLAGS = $(PISOCK_LDFLAGS) $(all_libraries) -module $(KDE_PLUGIN)
kfile_palm_la_LIBADD = $(LIB_KSYCOCA) $(PISOCK_LIB)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_palm.cpp -o $(podir)/kfile_palm.pot

services_DATA = kfile_palm.desktop
servicesdir = $(kde_servicesdir)
