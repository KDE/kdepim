INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/kresources/lib -I../lib -I$(top_srcdir)/kresources/blogging/libkblog/ -I$(top_srcdir)/libkdepim $(all_includes)

lib_LTLIBRARIES = libkcal_blogging.la
kde_module_LTLIBRARIES = kcal_blogging.la

libkcal_blogging_la_SOURCES = API_Blog.cpp API_Blogger.cpp xmlrpcjob.cpp \
  resourcebloggingsettings.ui kcal_resourcebloggingconfig.cpp \
  bloggingcalendaradaptor.cpp \
  kcal_resourceblogging.cpp 
libkcal_blogging_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) \
  -version-info 1:0:0 -no-undefined
libkcal_blogging_la_LIBADD = \
  ../lib/libkgroupwarebase.la \
  $(top_builddir)/libkcal/libkcal.la $(top_builddir)/libkdepim/libkdepim.la 


kcal_blogging_la_SOURCES = kcal_resourceblogging_plugin.cpp
kcal_blogging_la_LDFLAGS = $(all_libraries) -module -no-undefined $(KDE_PLUGIN)
kcal_blogging_la_LIBADD = libkcal_blogging.la

kcal_servicedir = $(kde_servicesdir)/kresources/kcal
kcal_service_DATA = blogging.desktop

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kres_blogging.pot

