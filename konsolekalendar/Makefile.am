INCLUDES= -I$(top_srcdir)/libkcal $(all_includes)

METASOURCES = AUTO

bin_PROGRAMS = konsolekalendar

konsolekalendar_SOURCES = kalendarVariables.cpp konsolekalendar.cpp main.cpp 
konsolekalendar_LDADD   = -lkcal $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)
konsolekalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)
