INCLUDES= $(all_includes) -I$(LIBPISOCK_PATH) \
	-I../../lib

METASOURCES = AUTO

CXXFLAGS+=-DDEBUG -DDEBUG_CERR

servicedir = $(kde_servicesdir)
service_DATA = null-conduit.desktop

kde_module_LTLIBRARIES = libnullconduit.la


# Which sources should be compiled for vcalconduit
# null_conduit_SOURCES= null-conduit.cc setupDialog.cc
libnullconduit_la_SOURCES = null-factory.cc setup_base.ui
libnullconduit_la_LDFLAGS = $(KDE_RPATH) -L../../lib
libnullconduit_la_LIBADD = -lkpilot

