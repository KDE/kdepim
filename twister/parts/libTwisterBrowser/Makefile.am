KDE_CXXFLAGS = -DQT_NO_ASCII_CAST -DKPARTS_VERSION

INCLUDES = -I$(top_srcdir)/twister/lib $(all_includes)

lib_LTLIBRARIES = libTwisterBrowser.la

libTwisterBrowser_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module -no-undefined -DKPARTS_VERSION

libTwisterBrowser_la_LIBADD = $(LIB_KPARTS) $(LIB_KDEUI) $(LIB_KDECORE) \
  $(top_builddir)/twister/lib/libtwisterkernel.la

METASOURCES = AUTO

libTwisterBrowser_la_SOURCES = \
    TwisterSideBar.cpp \
    TwisterPartHolder.cpp \
    TwisterBrowser.cpp

noinst_HEADERS = TwisterBrowser.h

servicedir = $(kde_servicesdir)
service_DATA = TwisterBrowser.desktop

rcdir = $(kde_datadir)/TwisterBrowser
rc_DATA = TwisterBrowser.rc
