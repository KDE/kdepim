INCLUDES  = -I$(top_srcdir)/kab2/lib $(all_includes)
LDFLAGS   = -L$(top_builddir)/kab2/lib $(all_libraries) $(KDE_RPATH)
LDADD = -lKAddressBook -lkdecore -lqt

bin_PROGRAMS = kab_server

KAddressBookInterface_DIR = $(top_srcdir)/kab2/lib
kab_server_SOURCES = KAddressBook.cpp KAddressBookInterface.skel main.cpp

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kab_server.desktop

