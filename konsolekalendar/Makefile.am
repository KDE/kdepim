INCLUDES= -I$(top_srcdir) $(all_includes)

METASOURCES = AUTO

bin_PROGRAMS = konsolekalendar

konsolekalendar_SOURCES = konsolekalendarepoch.cpp konsolekalendardelete.cpp \
  konsolekalendarchange.cpp konsolekalendarvariables.cpp \
  konsolekalendaradd.cpp konsolekalendarexports.cpp konsolekalendar.cpp \
  main.cpp 
konsolekalendar_LDADD = $(top_builddir)/libkcal/libkcal.la 
konsolekalendar_LDFLAGS = $(all_libraries) $(KDE_RPATH)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/konsolekalendar.pot

include ../admin/Doxyfile.am
