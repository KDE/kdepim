
SUBDIRS = lib overview addressbook organizer debugger

INCLUDES = -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector2 -I$(top_srcdir)/kitchensync/kitchensync/lib $(all_includes)

kitchensyncdatadir = $(kde_datadir)/kitchensync
kitchensyncdata_DATA = ksyncgui.rc

METASOURCES = AUTO

bin_PROGRAMS = kitchensync

kitchensync_SOURCES = main.cpp splash.cpp

kitchensync_LDADD =  ../libkonnector2/libkonnector.la ./lib/libkitchensyncui.la -lkparts
kitchensync_LDFLAGS = $(KDE_RPATH) $(all_libraries)
