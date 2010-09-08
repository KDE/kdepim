INCLUDES = -I$(top_srcdir)/kitchensync/src \
           -I$(top_srcdir)/kitchensync \
           -I$(top_srcdir) \
           $(OPENSYNC_CFLAGS) \
           $(OPENSYNCENGINE_CFLAGS) \
           $(all_includes)

lib_LTLIBRARIES = libqopensync.la

libqopensync_la_SOURCES = callbackhandler.cpp conversion.cpp engine.cpp environment.cpp filter.cpp group.cpp \
	member.cpp plugin.cpp result.cpp syncmapping.cpp syncupdates.cpp \
	syncchange.cpp
libqopensync_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libqopensync_la_LIBADD = $(LIB_KDEUI) $(OPENSYNC_LIBS) $(OPENSYNCENGINE_LIBS)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/libqopensync.pot
