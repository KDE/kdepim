INCLUDES = -I$(top_srcdir)/libkcal $(all_includes)

lib_LTLIBRARIES   = libksync.la
libksync_la_LDFLAGS =  $(all_libraries) -no-undefined -version-info 0:0:0 -module
libksync_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la -lkabc $(LIB_QT)

libksync_la_SOURCES = \
  syncer.cpp syncui.cpp syncuikde.cpp kontainer.cpp \
  syncalgorithm.cpp standardsync.cpp \
  addressbooksyncee.cpp unknownsyncee.cpp
 

METASOURCES = AUTO
