KDE_CXXFLAGS = -DQT_NO_ASCII_CAST

INCLUDES = $(all_includes) -I$(top_srcdir)/twister/lib

lib_LTLIBRARIES = libtwisterui.la

libtwisterui_la_LDFLAGS = $(all_libraries) -version-info 1:0:0 -module
#	Define this for testing
#	-no-undefined

rcdir = $(kde_datadir)/twister
rc_DATA = TwisterShell.rc

libtwisterui_la_LIBADD = \
  $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDECORE)

METASOURCES = AUTO

libtwisterui_la_SOURCES = TwisterShell.cpp TwisterUI.cpp
