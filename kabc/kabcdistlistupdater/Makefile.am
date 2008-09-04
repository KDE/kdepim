INCLUDES = -I$(includedir)/kabc -I$(top_srcdir) $(all_includes)

METASOURCES = AUTO

bin_PROGRAMS = kabcdistlistupdater

kabcdistlistupdater_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kabcdistlistupdater_LDADD = $(LIB_KABC) $(top_builddir)/libkdepim/libkdepim.la
kabcdistlistupdater_SOURCES = kabcdistlistupdater.cpp

autostart_DATA = kabcdistlistupdater.desktop
autostartdir = $(datadir)/autostart
