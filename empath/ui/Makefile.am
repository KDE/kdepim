KDE_CXXFLAGS = $(LIBQPTHREAD_CXXFLAGS) -DQT_NO_ASCII_CAST

INCLUDES = \
	-I$(top_srcdir)/empath/lib \
	-I$(top_srcdir)/librmm \
	$(all_includes)

lib_LTLIBRARIES = libempathui.la

libempathui_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module
#	Define this for testing
#	-no-undefined

rcdir = $(kde_datadir)/empath
rc_DATA = EmpathMainWindow.rc

libempathui_la_LIBADD = \
  $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDECORE) \
	$(top_builddir)/empath/lib/libempathkernel.la

METASOURCES = AUTO

libempathui_la_SOURCES = \
EmpathMainWindow.cpp \
EmpathUI.cpp
