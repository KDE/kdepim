KDE_CXXFLAGS = -DQT_NO_ASCII_CAST -DKPARTS_VERSION

INCLUDES = $(all_includes) \
  -I$(top_srcdir)/empath/lib \
  -I$(top_srcdir)/empath/ui/libEmpathListView

lib_LTLIBRARIES = libEmpathComposeWidget.la

libEmpathComposeWidget_la_LDFLAGS = \
	$(all_libraries) -version-info 1:0:0 -module -DKPARTS_VERSION

libEmpathComposeWidget_la_LIBADD = \
	$(LIB_KPARTS) $(LIB_KDEUI) $(LIB_KDECORE) \
  $(top_builddir)/empath/ui/libEmpathListView/libEmpathListView.la

METASOURCES = AUTO

libEmpathComposeWidget_la_SOURCES = \
EmpathAttachmentEditDialog.cpp \
EmpathAttachmentListItem.cpp \
EmpathAttachmentListWidget.cpp \
EmpathComposeWidget.cpp \
EmpathEditorProcess.cpp \
EmpathEnvelopeWidget.cpp \
EmpathHeaderSpecWidget.cpp

noinst_HEADERS = \
EmpathAttachmentEditDialog.h \
EmpathAttachmentListItem.h \
EmpathAttachmentListWidget.h \
EmpathComposeWidget.h \
EmpathEditorProcess.h \
EmpathEnvelopeWidget.h \
EmpathHeaderBodyWidget.h \
EmpathHeaderSpecWidget.h

servicedir = $(kde_servicesdir)
service_DATA = EmpathComposeWidget.desktop

rcdir = $(kde_datadir)/EmpathComposeWidget
rc_DATA = EmpathComposeWidget.rc

EXTRA_DIST = $(top_srcdir)/empath/lib/EmpathComposeForm.h
