KDE_CXXFLAGS = -DQT_NO_ASCII_CAST

INCLUDES = $(all_includes) -I$(top_srcdir)/empath/lib -I$(top_srcdir)/empath/ui

lib_LTLIBRARIES = libEmpathMessageListWidget.la

libEmpathMessageListWidget_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module

libEmpathMessageListWidget_la_LIBADD = $(LIB_KDEUI) $(LIB_KDECORE) $(top_builddir)/empath/ui/EmpathListView.lo $(top_builddir)/empath/lib/EmpathIndexRecord.lo

METASOURCES = AUTO

libEmpathMessageListWidget_la_SOURCES = \
EmpathMessageListItem.cpp \
EmpathMessageListWidget.cpp \
EmpathMessageMarkDialog.cpp

EXTRA_DIST = \
  $(top_srcdir)/empath/lib/EmpathIndexRecord.h \
  $(top_srcdir)/empath/ui/EmpathListView.h
