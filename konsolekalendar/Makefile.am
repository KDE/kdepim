INCLUDES= -I$(top_srcdir)/libkcal $(all_includes)

METASOURCES = AUTO

bin_PROGRAMS = konsolekalendar

konsolekalendar_SOURCES = konsolekalendarvariables.cpp konsolekalendaradd.cpp konsolekalendar.cpp main.cpp 
konsolekalendar_LDADD   = $(top_builddir)/libkcal/libkcal.la $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)
konsolekalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/konsolekalendar.pot

include ../admin/Doxyfile.am
