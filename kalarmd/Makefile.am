INCLUDES= -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkalarmd.la

libkalarmd_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkalarmd_la_LIBADD  = $(LIB_KDECORE)
libkalarmd_la_SOURCES =  alarmdaemoniface.stub alarmguiiface.stub dummy.cpp

DISTCLEANFILES = dummy.cpp

dummy.cpp:
	touch dummy.cpp


bin_PROGRAMS = kalarmd kalarmdgui

kalarmd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kalarmd_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la libkalarmd.la
kalarmd_SOURCES = alarmdaemon.cpp main.cpp alarmapp.cpp \
        adcalendarbase.cpp adcalendar.cpp adconfigdatabase.cpp calclient.cpp \
        adconfigdatarw.cpp \
        alarmdaemoniface.skel

kalarmdgui_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kalarmdgui_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la libkalarmd.la
kalarmdgui_SOURCES = guimain.cpp guiapp.cpp alarmdialog.cpp \
	dockwindow.cpp \
	koeventviewer.cpp alarmgui.cpp adconfigdatabase.cpp \
        calclient.cpp \
        adcalendarbase.cpp adcalendar_gui.cpp \
        alarmguiiface.skel


kde_module_LTLIBRARIES = libkcm_alarmdaemonctrl.la

libkcm_alarmdaemonctrl_la_SOURCES = alarmdaemonctrl.cpp
libkcm_alarmdaemonctrl_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_alarmdaemonctrl_la_LIBADD = $(LIB_KDEUI)

alarmdaemonctrl_DATA = alarmdaemonctrl.desktop
alarmdaemonctrldir = $(kde_appsdir)/Settings/System


noinst_HEADERS = alarmapp.h alarmdaemon.h alarmdaemoniface.h \
	guiapp.h alarmgui.h alarmguiiface.h alarmdialog.h \
	dockwindow.h koeventviewer.h \
        adcalendarbase.h adcalendar.h adcalendar_gui.h adconfigdatabase.h

METASOURCES = AUTO

SUBDIRS = pixmaps
 
autostart_DATA = kalarmd.autostart.desktop kalarmdgui.desktop
autostartdir = $(prefix)/share/autostart

#datadir = $(kde_bindir)

KDE_ICON = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kalarmdgui.pot
