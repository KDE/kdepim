KDE_CXXFLAGS = -DQT_NO_ASCII_CAST -DKPARTS_VERSION

INCLUDES = $(all_includes) \
  -I$(top_srcdir)/empath/lib \
  -I$(top_srcdir)/empath/ui \
  -I$(top_srcdir)/librmm

lib_LTLIBRARIES = libEmpathComposePart.la

libEmpathComposePart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)

libEmpathComposePart_la_LIBADD = \
	$(LIB_KPARTS) $(LIB_KDEUI) $(LIB_KDECORE) \
  $(top_builddir)/empath/ui/libempathui.la

METASOURCES = AUTO

libEmpathComposePart_la_SOURCES = \
EmpathAttachmentEditDialog.cpp \
EmpathAttachmentListItem.cpp \
EmpathAttachmentListWidget.cpp \
EmpathComposeWidget.cpp \
EmpathEditorProcess.cpp \
EmpathEnvelopeWidget.cpp \
EmpathHeaderSpecWidget.cpp \
EmpathHeaderBodyWidget.cpp \
EmpathAddressHeaderBodyWidget.cpp \
EmpathTextHeaderBodyWidget.cpp

noinst_HEADERS = \
EmpathAttachmentEditDialog.h \
EmpathAttachmentListItem.h \
EmpathAttachmentListWidget.h \
EmpathComposeWidget.h \
EmpathEditorProcess.h \
EmpathEnvelopeWidget.h \
EmpathHeaderBodyWidget.h \
EmpathTextHeaderBodyWidget.h \
EmpathAddressHeaderBodyWidget.h \
EmpathHeaderSpecWidget.h

servicedir = $(kde_servicesdir)
service_DATA = EmpathComposePart.desktop

rcdir = $(kde_datadir)/EmpathComposePart
rc_DATA = EmpathComposePart.rc

EXTRA_DIST = $(top_srcdir)/empath/lib/EmpathComposeForm.h
