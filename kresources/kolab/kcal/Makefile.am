METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kresources/kolab/shared -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kcal_kolab.la

noinst_HEADERS = resourcekolab.h incidence.h event.h task.h

kcal_kolab_la_SOURCES = resourcekolab.cpp incidence.cpp event.cpp task.cpp
kcal_kolab_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kcal_kolab_la_LIBADD =  $(top_builddir)/libkcal/libkcal.la \
	$(top_builddir)/kresources/kolab/shared/libresourcekolabshared.la \
	$(top_builddir)/libkdepim/libkdepim.la -lkresources

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcal_kolab.pot

servicedir = $(kde_servicesdir)/kresources/kcal
service_DATA = kolab.desktop
