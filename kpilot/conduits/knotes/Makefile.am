INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH) \
	-I../../lib -DDEBUG -DDEBUG_CERR

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = knotes-conduit.desktop

kde_module_LTLIBRARIES = libknotesconduit.la


libknotesconduit_la_SOURCES = knotes-factory.cc \
	knotes-setup.cc knotes-action.cc \
	setup_base.ui \
	KNotesIface.stub
libknotesconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libknotesconduit_la_LIBADD = ../../lib/libkpilot.la

