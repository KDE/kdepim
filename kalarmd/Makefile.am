# set the include path for X, qt and KDE, and versit library for vCalendar
INCLUDES= $(all_includes) -I$(top_srcdir)/libkcal

bin_PROGRAMS = kalarmd kalarmdgui
kalarmd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kalarmd_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la
kalarmd_SOURCES = alarmdaemon.cpp main.cpp alarmapp.cpp \
	alarmdaemoniface.skel alarmdaemoniface.stub \
        adcalendarbase.cpp adcalendar.cpp adconfigdatabase.cpp calclient.cpp \
        adconfigdatarw.cpp

kalarmdgui_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kalarmdgui_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la
kalarmdgui_SOURCES = guimain.cpp guiapp.cpp alarmdialog.cpp \
	dockwindow.cpp daemonguiiface.skel daemonguiiface.stub \
	koeventviewer.cpp daemongui.cpp

noinst_HEADERS = alarmapp.h alarmdaemon.h alarmdaemoniface.h \
	guiapp.h daemongui.h daemonguiiface.h alarmdialog.h \
	dockwindow.h koeventviewer.h \
        acalendarbase.cpp adcalendar.h

METASOURCES = AUTO

SUBDIRS = pixmaps
 
autostart_DATA = kalarmd.desktop kalarmdgui.desktop
autostartdir = $(prefix)/share/autostart

#datadir = $(kde_bindir)

kde_conf_DATA = kalarmdrc kalarmdguirc

KDE_ICON = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kalarmdgui.pot
