KDE_CXXFLAGS = -DKPARTS_VERSION

INCLUDES = $(all_includes) \
  -I$(top_srcdir)/empath/lib \
  -I$(top_srcdir)/empath/ui

lib_LTLIBRARIES = libEmpathView.la

libEmpathView_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

libEmpathView_la_LIBADD = \
	$(LIB_KPARTS) \
  $(top_builddir)/empath/ui/libempathui.la

libEmpathView_la_METASOURCES = AUTO

libEmpathView_la_SOURCES = EmpathView.cpp

noinst_HEADERS = EmpathView.h

servicedir = $(kde_servicesdir)
service_DATA = EmpathView.desktop

rcdir = $(kde_datadir)/EmpathView
rc_DATA = EmpathView.rc

