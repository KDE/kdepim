bin_PROGRAMS = kandy kandy_client

INCLUDES = -I$(top_srcdir) $(all_includes)

kandy_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kandy_LDADD   = $(KANDY_LIBLOCKDEV) $(LIB_KFILE) -lkabc $(top_builddir)/libkdepim/libkdepim.la
kandy_SOURCES =	main.cpp kandy.cpp \
                cmdpropertiesdialog_base.ui cmdpropertiesdialog.cpp \
                kandyview.cpp \
		kandyiface.skel \
                modem.cpp \
                atcommand.cpp commanditem.cpp mobilegui_base.ui mobilegui.cpp \
                commandscheduler.cpp commandset.cpp \
                kandyprefs.kcfgc kandyprefsdialog.cpp \
                mobilemain.cpp

noinst_HEADERS   = kandy.h kandyview.h modem.h atcommand.h \
                   commanditem.h mobilegui.h commandscheduler.h

kandy_client_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kandy_client_LDADD   = $(LIB_KDECORE)
kandy_client_SOURCES = kandy_client.cpp

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kandy.pot

xdg_apps_DATA = kandy.desktop

kde_kcfg_DATA = kandy.kcfg

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Utilities
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Utilities/kandy.desktop

rcdir = $(kde_datadir)/kandy
rc_DATA = kandyui.rc kandymobileui.rc
