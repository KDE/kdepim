KDE_CXXFLAGS = -DQT_NO_ASCII_CAST -DKPARTS_VERSION

INCLUDES = -I$(top_srcdir)/librmm -I$(top_srcdir)/empath/lib $(all_includes)

lib_LTLIBRARIES = libEmpathMessageViewWidget.la

libEmpathMessageViewWidget_la_LDFLAGS = $(all_libraries) $(KDE_MODULE)

libEmpathMessageViewWidget_la_LIBADD = \
	$(LIB_KPARTS) \
	$(top_builddir)/empath/lib/libempathkernel.la \
	$(top_builddir)/librmm/librmm.la

METASOURCES = AUTO

libEmpathMessageViewWidget_la_SOURCES = \
EmpathMessageHeaderViewWidget.cpp \
EmpathMessageTextViewWidget.cpp \
EmpathMessageAttachmentViewWidget.cpp \
EmpathMessageViewWidget.cpp

noinst_HEADERS = \
EmpathMessageAttachmentViewWidget.h \
EmpathMessageHeaderViewWidget.h \
EmpathMessageTextViewWidget.h \
EmpathMessageViewWidget.h

servicedir = $(kde_servicesdir)
service_DATA = EmpathMessageViewWidget.desktop

rcdir = $(kde_datadir)/EmpathMessageViewWidget
rc_DATA = EmpathMessageViewWidget.rc
