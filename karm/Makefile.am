
############ Autoconf-generated variables  ###################
AM_CPPFLAGS = -DQT_NO_CAST_ASCII
INCLUDES = $(all_includes)
LDFLAGS	= $(all_libraries) $(KDE_RPATH)
LDADD =  -lkdeprint $(LIBXSS)


bin_PROGRAMS = karm
SUBDIRS = support pics

METASOURCES = AUTO

karm_SOURCES =  adddlg.cpp main.cpp \
				task.cpp top.cpp kaccelmenuwatch.cpp print.cpp \
				preferences.cpp ktimewidget.cpp karm.cpp idle.cpp
noinst_HEADERS = adddlg.h karm.h task.h toolicons.h top.h kaccelmenuwatch.h \
				 version.h print.h idle.h preferences.h ktimewidget.h

KDE_ICON = karm

appsdir = 

rcdir = $(kde_datadir)/karm
rc_DATA = karmui.rc

lnk_DATA = support/karm.desktop
lnkdir = $(kde_appsdir)/Utilities/More

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(kde_appsdir)/Utilities
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(kde_appsdir)/Utilities/karm.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/karm.pot

srcdoc:
	kdoc -a -p -H -d $(HOME)/web/src/karm karm *.h -lkdecore -lkdeui -lqt


