
############ Autoconf-generated variables  ###################
AM_CPPFLAGS = -DQT_NO_CAST_ASCII
INCLUDES = -I$(top_srcdir)/libkcal $(all_includes)

bin_PROGRAMS = karm
SUBDIRS = support pics

METASOURCES = AUTO

karm_SOURCES =  desktoptracker.cpp edittaskdialog.cpp idletimedetector.cpp \
		kaccelmenuwatch.cpp karmutility.cpp ktimewidget.cpp \
		logging.cpp main.cpp \
		mainwindow.cpp preferences.cpp print.cpp task.cpp \
		taskview.cpp tray.cpp
karm_LDADD = -lkdeprint $(top_builddir)/libkcal/libkcal.la $(LIBXSS)
karm_LDFLAGS = $(all_libraries)
noinst_HEADERS = desktoptracker.h edittaskdialog.h \
                 idletimedetector.h kaccelmenuwatch.h \
		 karmutility.h ktimewidget.h logging.h mainwindow.h \
		 preferences.h print.h task.h taskview.h toolicons.h \
		 tray.h version.h

KDE_ICON = karm

KDE_OPTIONS = nofinal

rcdir = $(kde_datadir)/karm
rc_DATA = karmui.rc

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Utilities
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Utilities/karm.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/karm.pot

srcdoc:
	kdoc -a -p -H -d $(HOME)/web/src/karm karm *.h -lkdecore -lkdeui -lqt

include ../admin/Doxyfile.am
