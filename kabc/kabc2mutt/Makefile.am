INCLUDES = -I$(includedir)/kabc $(all_includes)

METASOURCES = AUTO

bin_PROGRAMS = kabc2mutt

kabc2mutt_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kabc2mutt_LDADD = -lkabc $(LIB_KDECORE)
kabc2mutt_SOURCES = kabc2mutt.cpp main.cpp

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kabc2mutt.pot

include $(top_srcdir)/doc/api/Doxyfile.am
