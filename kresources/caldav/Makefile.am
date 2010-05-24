INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkcal_caldav.la

libkcal_caldav_la_SOURCES = resource.cpp \
                               config.cpp \
                               configwidgets.cpp \
                               preferences.cpp \
                               job.cpp \
                               reader.cpp \
                               writer.cpp \
                               prefsskel.kcfgc
libkcal_caldav_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_caldav_la_LIBADD = \
                              $(top_builddir)/libkcal/libkcal.la \
                              $(top_builddir)/libkdepim/libkdepim.la \
                              -lcaldav
libkcal_caldav_la_COMPILE_FIRST = prefsskel.h

kde_module_LTLIBRARIES = kcal_caldav.la

kcal_caldav_la_SOURCES = plugin.cpp
kcal_caldav_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_caldav_la_LIBADD = libkcal_caldav.la
kcal_caldav_la_COMPILE_FIRST = prefsskel.h

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_caldav.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_caldav.pot
