INCLUDES  = -I$(top_srcdir)/kab2/lib $(all_includes)
LDFLAGS   = -L$(top_srcdir)/kab2/lib $(all_libraries) $(KDE_RPATH)
LDADD = -lkab2 -lkdecore -lqt

bin_PROGRAMS = kab_server

kab_server_SOURCES = KAddressBook.cpp main.cpp

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kab_server.desktop

