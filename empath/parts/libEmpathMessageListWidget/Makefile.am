KDE_CXXFLAGS = -DQT_NO_ASCII_CAST -DKPARTS_VERSION

INCLUDES = $(all_includes) -I$(top_srcdir)/empath/lib

lib_LTLIBRARIES = libEmpathMessageListWidget.la

libEmpathMessageListWidget_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module -no-undefined -DKPARTS_VERSION

libEmpathMessageListWidget_la_LIBADD = \
	$(LIB_KPARTS) $(LIB_KDEUI) $(LIB_KDECORE)

METASOURCES = AUTO

libEmpathMessageListWidget_la_SOURCES = \
EmpathMessageMarkDialog.cpp \
EmpathListView.cpp \
EmpathMessageListItem.cpp \
EmpathMessageListWidget.cpp

noinst_HEADERS = \
EmpathMessageMarkDialog.h \
EmpathListView.h \
EmpathMessageListItem.h \
EmpathMessageListWidget.h

servicedir = $(kde_servicesdir)
service_DATA = EmpathMessageListWidget.desktop

rcdir = $(kde_datadir)/EmpathMessageListWidget
rc_DATA = EmpathMessageListWidget.rc

EXTRA_DIST = $(top_srcdir)/empath/lib/EmpathIndexRecord.h
