## Makefile.am for kmobile

SUBDIRS = . kioslave devices

bin_PROGRAMS = kmobile 

kde_module_LTLIBRARIES = libkmobiledevice.la libkmobileclient.la

INCLUDES         = -I$(top_builddir)/libkdepim $(all_includes)

kmobile_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kmobile_LDADD   = $(LIB_KFILE) $(LIB_KABC) libkmobiledevice.la
kmobile_SOURCES = main.cpp kmobile.cpp kmobileview.cpp kmobileitem.cpp\
			pref.cpp kmobileiface.skel kmobile_selectiondialog.ui \
			systemtray.cpp

KDE_ICON = kmobile

noinst_HEADERS = kmobile.h kmobileview.h kmobileitem.h pref.h

# the low-level devices driver library
libkmobiledevice_la_SOURCES = kmobiledevice.cpp
libkmobiledevice_la_LDFLAGS = $(all_libraries) -no-undefined -avoid-version -module
libkmobiledevice_la_LIBADD = $(LIB_KABC) $(LIB_LOCKDEV)

# the KDE application's client library to KMobile
libkmobileclient_la_SOURCES = kmobileclient.cpp
libkmobileclient_la_LDFLAGS = $(all_libraries) -no-undefined -avoid-version -module
libkmobileclient_la_LIBADD = $(LIB_KDECORE) -lDCOP

EXTRA_PROGRAMS = testlibkmobile
testlibkmobile_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testlibkmobile_LDADD = libkmobileclient.la
testlibkmobile_SOURCES = testlibkmobile.cpp

# the servicetype for mobile devices
kde_servicetypes_DATA = libkmobile.desktop

METASOURCES = AUTO

xdg_apps_DATA = kmobile.desktop

rc_DATA = kmobileui.rc
rcdir = $(kde_datadir)/kmobile

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kmobile.pot

