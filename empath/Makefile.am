APPSDIR = $(kde_appsdir)/Internet
INCLUDES= $(all_includes) -I rmm/include/ -I lib/
SUBDIRS = doc pics tips templates rmm lib ui

bin_PROGRAMS = 	empath

empath_LDADD = ui/libempathui.a lib/libempath.a rmm/librmm.a -lkhtml -lkimgio -ljpeg6b -lX11 -lm -ljscript -lkdeutil -lkfile -lkfm -lkdeui -lkdecore -lqt -lXext -lmimelib -lpng -ltiff -lz -ldl

empath_LDFLAGS = $(all_libraries) $(KDE_RPATH)

empath_SOURCES = main.cpp

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(empath_SOURCES) && mv messages.po ../po/empath.pot

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) empath.kdelnk $(APPSDIR)
	$(mkinstalldirs) $(kde_datadir)/empath
	
uninstall-local:
	-rm -f $(APPSDIR)/empath.kdelnk
	-rm -rf $(kde_datadir)/empath

