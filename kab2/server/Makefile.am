INCLUDES  = -I../lib $(all_includes)
LDFLAGS   = -L../lib $(all_libraries) $(KDE_RPATH)
LDADD = -lkab2 -ldl -lqt -lkdecore

bin_PROGRAMS = kab_server

kab_server_CXXFLAGS  = $(CXXFLAGS) -rdynamic

datadir = $(kde_datadir)/kab/server

kab_server_SOURCES = server.cpp
