
SUBDIRS = settings

INCLUDES = \
	-I$(top_srcdir)/empath/ui/settings \
	-I$(top_srcdir)/empath/lib \
	-I$(top_srcdir)/librmm \
	$(all_includes)

lib_LTLIBRARIES = libempathui.la

libempathui_la_LDFLAGS = $(all_libraries) \
    -version-info 1:0:0 -module -no-undefined

rcdir = $(kde_datadir)/empath
rc_DATA = EmpathMainWindow.rc

libempathui_la_LIBADD = \
	$(top_builddir)/empath/ui/settings/libEmpathSettingsDialog.la \
  $(LIB_KPARTS) $(LIB_KFILE) \
	$(top_builddir)/empath/lib/libempathkernel.la

libempathui_la_METASOURCES = AUTO

libempathui_la_SOURCES = \
EmpathFolderChooserDialog.cpp \
EmpathFolderChooserWidget.cpp \
EmpathFolderCombo.cpp \
EmpathFolderListItem.cpp \
EmpathFolderListWidget.cpp \
EmpathListView.cpp \
EmpathMainWindow.cpp \
EmpathMessageListItem.cpp \
EmpathMessageListWidget.cpp \
EmpathMessageMarkDialog.cpp \
EmpathUI.cpp

