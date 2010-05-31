INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkabc_carddav.la

libkabc_carddav_la_SOURCES = resource.cpp \
                               config.cpp \
                               configwidgets.cpp \
                               preferences.cpp \
                               job.cpp \
                               reader.cpp \
                               writer.cpp \
                               prefsskel.kcfgc
libkabc_carddav_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkabc_carddav_la_LIBADD = \
                              $(top_builddir)/libkdepim/libkdepim.la \
                              $(top_builddir)/libkcal/libkcal.la \
                              -lkabc \
                              -lcarddav
libkabc_carddav_la_COMPILE_FIRST = prefsskel.h

kde_module_LTLIBRARIES = kabc_carddav.la

kabc_carddav_la_SOURCES = plugin.cpp
kabc_carddav_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
kabc_carddav_la_LIBADD = libkabc_carddav.la
kabc_carddav_la_COMPILE_FIRST = prefsskel.h

kabc_servicedir = $(kde_servicesdir)/kresources/kabc
kabc_service_DATA = kabc_carddav.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_carddav.pot
