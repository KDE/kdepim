bin_PROGRAMS = ksync
ksync_SOURCES = ksyncview.cpp ksync.cpp main.cpp 
ksync_LDADD   = ../lib/libksync.la $(LIB_KIO)
ksync_LDFLAGS = $(all_libraries) $(KDE_RPATH)

EXTRA_DIST = main.cpp ksync.cpp ksync.h ksyncdoc.cpp ksyncdoc.h ksyncview.cpp ksyncview.h ksyncui.rc ksync.desktop lo32-app-ksync.png lo16-app-ksync.png 

INCLUDES = -I$(top_srcdir)/ksync/lib -I$(top_srcdir)/libkcal $(all_includes)

METASOURCES = AUTO


rcdir = $(kde_datadir)/ksync
rc_DATA = ksyncui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ksync.pot
