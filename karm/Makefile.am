
############ Autoconf-generated variables  ###################
AM_CPPFLAGS = -DQT_NO_CAST_ASCII
INCLUDES = -I$(top_srcdir)/libkcal $(all_includes)

bin_PROGRAMS = karm
SUBDIRS = support pics

METASOURCES = AUTO

karm_SOURCES =  addtaskdialog.cpp main.cpp \
		task.cpp mainwindow.cpp kaccelmenuwatch.cpp print.cpp \
		preferences.cpp ktimewidget.cpp taskview.cpp idletimedetector.cpp logging.cpp karmutility.cpp tray.cpp
karm_LDADD = -lkdeprint $(top_builddir)/libkcal/libkcal.la $(LIBXSS)
karm_LDFLAGS = $(all_libraries)
noinst_HEADERS = addtaskdialog.h taskview.h task.h toolicons.h mainwindow.h kaccelmenuwatch.h \
		version.h print.h idletimedetector.h preferences.h ktimewidget.h logging.h karmutility.h \
		tray.h

KDE_ICON = karm

rcdir = $(kde_datadir)/karm
rc_DATA = karmui.rc

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Utilities
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Utilities/karm.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/karm.pot

srcdoc:
	kdoc -a -p -H -d $(HOME)/web/src/karm karm *.h -lkdecore -lkdeui -lqt

include $(top_srcdir)/admin/Doxyfile.am
