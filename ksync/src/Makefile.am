bin_PROGRAMS = ksync
ksync_SOURCES = ksyncview.cpp ksync.cpp main.cpp 
ksync_LDADD   =  -lkfile -lkdeui -lkdecore $(LIB_QT) $(LIBSOCKET)
ksync_LDFLAGS = ../lib/libksync.la $(all_libraries) $(KDE_RPATH)

EXTRA_DIST = main.cpp ksync.cpp ksync.h ksyncdoc.cpp ksyncdoc.h ksyncview.cpp ksyncview.h ksyncui.rc ksync.desktop lo32-app-ksync.png lo16-app-ksync.png 

INCLUDES = -I$(top_srcdir)/ksync/lib -I$(top_srcdir)/libkcal $(all_includes)

METASOURCES = AUTO


rcdir = $(kde_datadir)/ksync
rc_DATA = ksyncui.rc

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/ksync.pot; \
	fi

