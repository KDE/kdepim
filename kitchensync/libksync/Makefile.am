SUBDIRS = . tests test

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/libkcal -I$(top_builddir)/libkdepim $(all_includes)

lib_LTLIBRARIES   = libksync2.la
libksync2_la_LDFLAGS =  $(all_libraries) -no-undefined -version-info 0:0:0
libksync2_la_LIBADD  = $(top_builddir)/libkcal/libkcal.la -lkabc $(LIB_QT) \
                       $(top_builddir)/libkdepim/libkdepim.la

libksync2_la_SOURCES = \
  syncer.cpp syncee.cpp syncentry.cpp syncui.cpp syncuikde.cpp kontainer.cpp \
  syncalgorithm.cpp standardsync.cpp \
  addressbooksyncee.cpp unknownsyncee.cpp \
  opiedesktopsyncee.cpp \
  calendarsyncee.cpp \
  synceelist.cpp \
  merger.cpp addressbookmerger.cpp calendarmerger.cpp \
  bookmarksyncee.cpp \
  conflictdialog.cpp \
  synchistory.cpp

ksincludedir = $(includedir)/kitchensync
ksinclude_HEADERS = addressbookmerger.h addressbooksyncee.h  \
                    bookmarksyncee.h calendarmerger.h        \
                    calendarsyncee.h conflictdialog.h        \
                    kontainer.h merger.h opiedesktopsyncee.h \
                    standardsync.h syncalgorithm.h syncee.h  \
                    synceelist.h syncentry.h syncer.h        \
                    synchistory.h syncui.h syncuikde.h       \
                    syncuikdeplugin.h unknownsyncee.h

noinst_HEADERS    = debug.h

METASOURCES = AUTO
