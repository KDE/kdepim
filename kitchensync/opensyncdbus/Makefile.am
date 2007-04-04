INCLUDES = -I/usr/local/include/dbus-1.0/qt3 $(all_includes) \
  -I$(top_srcdir)/kitchensync

bin_PROGRAMS = opensyncdbus

opensyncdbus_SOURCES = dbusclient.cpp
opensyncdbus_LDADD = $(top_builddir)/kitchensync/libqopensync/libqopensync.la
opensyncdbus_LDFLAGS = -ldbus-1-qt3 -ldbus-1 $(all_libraries) $(KDE_RPATH)

METASOURCES = AUTO
