# set the include path for X, qt and KDE and calendar library
INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/libical/src/libical \
          -I$(top_builddir)/libical/src/libical $(all_includes)

bin_PROGRAMS = kalarm kalarmautostart
kalarm_SOURCES = editdlg.cpp alarmcalendar.cpp daemongui.cpp \
	main.cpp kalarmapp.cpp mainwindowbase.cpp mainwindow.cpp \
	messagewin.cpp msgevent.cpp \
	prefsettings.cpp prefs.cpp prefdlg.cpp \
	traywindow.cpp alarmguiiface.skel \
	fontcolour.cpp colourcombo.cpp datetime.cpp \
	alarmlistview.cpp spinbox2.cpp
kalarm_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kalarm_LDADD   = $(LIB_KDEUI) $(top_builddir)/libkcal/libkcal.la $(top_builddir)/kalarmd/libkalarmd.la

kalarmautostart_SOURCES = autostart.cpp
kalarmautostart_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kalarmautostart_LDADD   = $(LIB_KDECORE)

noinst_HEADERS = kalarm.h kalarmapp.h mainwindow.h messagewin.h editdlg.h \
	mainwindowbase.h traywindow.h daemongui.h alarmguiiface.h \
	msgevent.h alarmcalendar.h alarmlistview.h datetime.h \
	fontcolour.h colourcombo.h \
	spinbox2.h prefs.h prefsettings.h prefdlg.h \
	autostart.h

METASOURCES = AUTO

SUBDIRS = pixmaps 

alarmguiiface.h: $(top_srcdir)/kalarmd/alarmguiiface.h
	cp $(top_srcdir)/kalarmd/alarmguiiface.h .

autostart_DATA = kalarm.tray.desktop
autostartdir = $(prefix)/share/autostart

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/kalarm.desktop $(kde_appsdir)/Applications/kalarm.desktop

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/kalarm.desktop

KDE_ICON = AUTO

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/kalarm.pot; \
	fi
