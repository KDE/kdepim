INCLUDES = -I$(top_srcdir)/kmobile -I$(top_srcdir)/libkdepim -I$(top_srcdir) $(all_includes) 

METASOURCES = AUTO

kde_module_LTLIBRARIES = libkmobile_skeleton.la 

kde_services_DATA = libkmobile_skeleton.desktop

libkmobile_skeleton_la_SOURCES = skeleton.cpp
libkmobile_skeleton_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) $(KDE_RPATH) -no-undefined -module
libkmobile_skeleton_la_LIBADD = ../../libkmobiledevice.la
