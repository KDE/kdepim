KDE_CXXFLAGS = -DLIBKLEOPATRA_NO_COMPAT

INCLUDES = -I$(top_srcdir)/certmanager/lib -I$(top_srcdir)/libkdenetwork -I$(top_srcdir)/libkdepim -I$(top_builddir)/libkdepim $(GPGME_CFLAGS) $(all_includes)

bin_PROGRAMS = kwatchgnupg

kwatchgnupg_SOURCES = \
	kwatchgnupgmainwin.cpp \
	kwatchgnupgconfig.cpp \
	aboutdata.cpp \
	tray.cpp \
	main.cpp

METASOURCES = AUTO

kwatchgnupg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kwatchgnupg_LDADD   = ../conf/libconf.la ../lib/libkleopatra.la $(LIB_KUTILS) $(LIB_KABC)

rcdir = $(kde_datadir)/kwatchgnupg
rc_DATA = kwatchgnupgui.rc


pics_DATA = kwatchgnupg.png kwatchgnupg2.png

picsdir = $(kde_datadir)/kwatchgnupg/pics

EXTRA_DIST = $(pics_DATA)

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kwatchgnupg.pot
