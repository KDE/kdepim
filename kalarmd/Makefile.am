# set the include path for X, qt and KDE, and versit library for vCalendar
INCLUDES= $(all_includes) -I$(top_srcdir)/libkcal

bin_PROGRAMS = kalarmd
kalarmd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kalarmd_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la
kalarmd_SOURCES = alarmdaemon.cpp main.cpp alarmapp.cpp alarmdialog.cpp \
	dockwindow.cpp alarmdaemoniface.skel alarmdaemoniface.stub \
	koeventviewer.cpp
noinst_HEADERS = alarmapp.h alarmdaemon.h alarmdaemoniface.h alarmdialog.h \
	dockwindow.h

METASOURCES = AUTO

SUBDIRS = pixmaps
 
autostart_DATA = kalarmd.desktop
autostartdir = $(prefix)/share/autostart

#datadir = $(kde_bindir)

kde_conf_DATA = kalarmdrc

KDE_ICON = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kalarmd.pot
