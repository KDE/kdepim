SUBDIRS = . test

KDE_OPTIONS = nofinal

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkcal $(all_includes)

lib_LTLIBRARIES   = libksync2.la
libksync2_la_LDFLAGS =  $(all_libraries) -no-undefined -version-info 0:0:0
libksync2_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la -lkabc $(LIB_QT)

libksync2_la_SOURCES = \
  syncer.cpp syncee.cpp syncentry.cpp syncui.cpp syncuikde.cpp kontainer.cpp \
  syncalgorithm.cpp standardsync.cpp \
  addressbooksyncee.cpp unknownsyncee.cpp \
  opiedesktopsyncee.cpp  todosyncee.cpp \
  eventsyncee.cpp \
  calendarsyncee.cpp \
  synceelist.cpp
 

METASOURCES = AUTO
