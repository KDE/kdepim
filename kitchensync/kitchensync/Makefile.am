
SUBDIRS = lib overview organizer addressbook

INCLUDES = -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector -I$(top_srcdir)/kitchensync/kitchensync/lib $(all_includes)

kitchensyncdatadir = $(kde_datadir)/kitchensync
kitchensyncdata_DATA = ksyncgui.rc

METASOURCES = AUTO

bin_PROGRAMS = kitchensync

kitchensync_SOURCES = 	ksync_main.cc \
	ksync_splash.cc

kitchensync_LDADD =  ../libkonnector/libkonnector.la    ./lib/libkitchensyncui.la -lkparts
kitchensync_LDFLAGS = $(KDE_RPATH) $(all_libraries)
