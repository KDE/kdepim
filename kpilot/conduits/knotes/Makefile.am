INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH) \
	-I../../lib

METASOURCES = AUTO

CXXFLAGS+=-DDEBUG -DDEBUG_CERR

servicedir = $(kde_servicesdir)
service_DATA = knotes-conduit.desktop

kde_module_LTLIBRARIES = libknotesconduit.la


libknotesconduit_la_SOURCES = knotes-factory.cc setup_base.ui \
	KNotesIface.stub
libknotesconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libknotesconduit_la_LIBADD = -lkpilot

