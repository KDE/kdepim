INCLUDES = -I$(top_srcdir)/kmobile -I$(top_srcdir) $(all_includes) 

METASOURCES = AUTO

kde_module_LTLIBRARIES = libkmobile_gnokii.la 

kde_services_DATA = libkmobile_gnokii.desktop

libkmobile_gnokii_la_SOURCES = gnokii_mobile.cpp gnokiiconfigui.ui gnokiiconfig.cpp
libkmobile_gnokii_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) $(KDE_RPATH) -no-undefined -module
libkmobile_gnokii_la_LIBADD = ../../libkmobiledevice.la ../../../libkcal/libkcal.la $(XPMLIB) $(LIB_GNOKII)
