KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS) -DQT_NO_ASCII_CAST -DKPARTS_VERSION

INCLUDES = -I$(top_srcdir)/empath/lib -I$(top_srcdir)/rmm $(all_includes)

lib_LTLIBRARIES = libEmpathBrowser.la

libEmpathBrowser_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module -no-undefined -DKPARTS_VERSION

libEmpathBrowser_la_LIBADD = $(LIB_KPARTS) $(LIB_KDEUI) $(LIB_KDECORE) \
  $(top_builddir)/empath/lib/libempathkernel.la \
  $(top_builddir)/rmm/librmm.la

METASOURCES = AUTO

libEmpathBrowser_la_SOURCES = EmpathBrowser.cpp

noinst_HEADERS = EmpathBrowser.h

servicedir = $(kde_servicesdir)
service_DATA = EmpathBrowser.desktop

rcdir = $(kde_datadir)/EmpathBrowser
rc_DATA = EmpathBrowser.rc
