INCLUDES = -I$(top_srcdir)/ktnef -I$(top_builddir)/libkdepim $(all_includes)

SUBDIRS = pics

bin_PROGRAMS = ktnef

ktnef_LDFLAGS = $(KDE_RPATH) $(all_libraries)
ktnef_LDADD   = $(LIB_KFILE) $(top_builddir)/ktnef/lib/libktnef.la
ktnef_SOURCES = ktnefmain.cpp ktnefview.cpp attachpropertydialogbase.ui \
                attachpropertydialog.cpp messagepropertydialog.cpp \
                qwmf.cpp main.cpp
ktnef_METASOURCES = AUTO

noinst_HEADERS = ktnefmain.h ktnefview.h attachpropertydialog.h \
                 qwmf.h wmfstruct.h metafuncs.h

rcdir = $(kde_datadir)/ktnef
rc_DATA = ktnefui.rc

#messages: rc.cpp
#	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(ktnef_SOURCES) && mv messages.po ../po/ktnef.pot

xdg_apps_DATA = ktnef.desktop

mime_DATA = ms-tnef.desktop
mimedir = $(kde_mimedir)/application
