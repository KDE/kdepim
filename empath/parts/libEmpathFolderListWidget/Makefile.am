KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS) -DQT_NO_ASCII_CAST -DKPARTS_VERSION

INCLUDES = -I$(top_srcdir)/empath/lib -I$(top_srcdir)/rmm \
  -I$(top_srcdir)/empath/parts/libEmpathListView $(all_includes)

lib_LTLIBRARIES = libEmpathFolderListWidget.la

libEmpathFolderListWidget_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module -no-undefined -DKPARTS_VERSION

libEmpathFolderListWidget_la_LIBADD = \
	$(LIB_KPARTS) $(LIB_KDEUI) $(LIB_KDECORE) \
    $(top_builddir)/empath/parts/libEmpathListView/libEmpathListView.la \
    $(top_builddir)/empath/lib/libempathkernel.la

METASOURCES = AUTO

libEmpathFolderListWidget_la_SOURCES = \
EmpathFolderChooserDialog.cpp \
EmpathFolderChooserWidget.cpp \
EmpathFolderListItem.cpp \
EmpathFolderListWidget.cpp

noinst_HEADERS = \
EmpathFolderChooserDialog.h \
EmpathFolderChooserWidget.h \
EmpathFolderListItem.h \
EmpathFolderListWidget.h

servicedir = $(kde_servicesdir)
service_DATA = EmpathFolderListWidget.desktop

rcdir = $(kde_datadir)/EmpathFolderListWidget
rc_DATA = EmpathFolderListWidget.rc
