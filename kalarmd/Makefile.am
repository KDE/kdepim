INCLUDES= -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES = libkalarmd.la

libkalarmd_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libkalarmd_la_LIBADD  = $(LIB_KDECORE)
libkalarmd_la_SOURCES =  alarmdaemoniface.stub alarmguiiface.stub dummy.cpp

DISTCLEANFILES = dummy.cpp

dummy.cpp:
	touch dummy.cpp

check_PROGRAMS = testalarmdlg

testalarmdlg_LDFLAGS = $(all_libraries) $(KDE_RPATH)
testalarmdlg_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la libkalarmd.la
testalarmdlg_SOURCES = testalarmdlg.cpp alarmdialog.cpp koeventviewer.cpp

bin_PROGRAMS = kalarmd

kalarmd_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kalarmd_LDADD = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la libkalarmd.la
kalarmd_SOURCES = alarmdaemon.cpp main.cpp alarmapp.cpp \
        adcalendarbase.cpp adcalendar.cpp adconfigdatabase.cpp clientinfo.cpp \
        adconfigdatarw.cpp \
        alarmdaemoniface.skel

noinst_HEADERS = alarmapp.h alarmdaemon.h alarmdaemoniface.h \
	guiapp.h alarmguiiface.h \
        adcalendarbase.h adcalendar.h adcalendar_gui.h \
	adconfigdatabase.h adconfigdatarw.h clientinfo.h

METASOURCES = AUTO
 
autostart_DATA = kalarmd.autostart.desktop
autostartdir = $(prefix)/share/autostart

app_DATA = kalarmd.desktop
appdir = $(kde_appsdir)/.hidden

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kalarmdgui.pot

include ../admin/Doxyfile.am
