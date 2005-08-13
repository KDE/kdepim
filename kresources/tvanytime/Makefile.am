INCLUDES = -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkcal_tvanytime.la

libkcal_tvanytime_la_SOURCES = kcal_resourcetvanytime.cpp \
		kcal_resourcetvanytimeconfig.cpp kcal_tvanytimeprefsbase.kcfgc 	kcal_tvanytimeprefs.cpp service.cpp

libkcal_tvanytime_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
                               -version-info 1:0:0 -no-undefined
libkcal_tvanytime_la_LIBADD = $(top_builddir)/libkcal/libkcal.la \
                              $(top_builddir)/libkdepim/libkdepim.la

kde_module_LTLIBRARIES = kcal_tvanytime.la

kcal_tvanytime_la_SOURCES = kcal_resourcetvanytime_plugin.cpp
kcal_tvanytime_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_tvanytime_la_LIBADD = libkcal_tvanytime.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = kcal_tvanytime.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(EXTRACTRC) `find . -name "*.ui" -o -name "*.kcfg"` >> rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_tvanytime.pot
noinst_HEADERS = service.h
