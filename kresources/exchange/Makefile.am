METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/korganizer/interfaces \
    -I$(top_srcdir)/interfaces \
    -I$(top_srcdir) \
    -I$(top_srcdir)/libkpimexchange/core -I$(top_builddir)/libkpimexchange/core \
    -I$(top_srcdir)/libical/src/libical -I$(top_srcdir)/libical/src/libicalss \
    -I$(top_builddir)/libical/src/libical -I$(top_builddir)/libical/src/libicalss \
    $(all_includes)

kde_module_LTLIBRARIES = resourcecalendarexchange.la 

resourcecalendarexchange_la_SOURCES = resourceexchange.cpp resourceexchangeconfig.cpp dateset.cpp
resourcecalendarexchange_la_LDFLAGS= $(all_libraries) -module $(KDE_PLUGIN)
resourcecalendarexchange_la_LIBADD= -lkresources \
                                    $(top_builddir)/libkcal/libkcal.la \
                                    $(top_builddir)/libkpimexchange/libkpimexchange.la 

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA= exchange.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_exchange.pot
