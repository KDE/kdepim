
SUBDIRS = organizer

INCLUDES = -I$(top_srcdir)/kitchensync/lib -I$(top_srcdir)/kitchensync/libkonnector $(all_includes)

kitchensyncdatadir = $(kde_datadir)/kitchensync
kitchensyncdata_DATA = ksyncgui.rc

METASOURCES = AUTO

bin_PROGRAMS = kitchensync

kitchensync_SOURCES = partbar.cc \
	ksync_mainwindow.cc manipulatorpart.cc \
	ksync_main.cc overviewwidget.cc ksync_configuredialog.cc

kitchensync_LDADD = ../lib/libksyncentries.la ../libkonnector/libkonnector.la -lkparts
kitchensync_LDFLAGS = $(KDE_RPATH) $(all_libraries)
