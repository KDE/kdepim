INCLUDES = -I$(includedir)/kabc $(all_includes)

METASOURCES = AUTO

bin_PROGRAMS = kabcfrontend

kabcfrontend_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kabcfrontend_LDADD = -lkabc $(LIB_KDECORE)
kabcfrontend_SOURCES = main.cpp mainWindow.ui

include $(top_srcdir)/doc/api/Doxyfile.am
