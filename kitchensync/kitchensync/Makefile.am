
SUBDIRS = lib overview organizer

INCLUDES = -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector -I$(top_srcdir)/kitchensync/kitchensync/lib $(all_includes)

kitchensyncdatadir = $(kde_datadir)/kitchensync
kitchensyncdata_DATA = ksyncgui.rc

METASOURCES = AUTO

bin_PROGRAMS = kitchensync

kitchensync_SOURCES = ksync_mainwindow.cc \
	ksync_main.cc \
	ksync_systemtray.cc \
	ksync_splash.cc

kitchensync_LDADD = ../lib/libksyncentries.la ../libkonnector/libkonnector.la ./organizer/liborganizerpart.la ./overview/liboverviewpart.la ./lib/libkitchensync.la -lkparts
kitchensync_LDFLAGS = $(KDE_RPATH) $(all_libraries)
