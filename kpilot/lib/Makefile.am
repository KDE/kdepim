METASOURCES = AUTO

INCLUDES = $(all_includes)

CXXFLAGS+=-DDEBUG -DDEBUG_CERR

kde_module_LTLIBRARIES = libkpilot.la

libkpilot_la_SOURCES = options.cc uiDialog.cc plugin.cc syncAction.cc \
	kpilotlink.cc
libkpilot_la_LIBADD = $(LIBPISOCK)
