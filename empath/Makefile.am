APPSDIR = $(kde_appsdir)/Internet
INCLUDES= $(all_includes) -I rmm/include/ -I lib/
SUBDIRS = doc pics toolbar tips templates rmm lib ui

bin_PROGRAMS = empath

empath_SOURCES = main.cpp

empath_LDADD = \
	./ui/libempathui.a ./lib/libempath.a ./rmm/librmm.a \
	-lkhtml -lkfile -lkdeui -lkdecore -lqt

empath_LDFLAGS = $(all_libraries) $(KDE_RPATH)

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(empath_SOURCES) && \
	mv messages.po ../po/empath.pot

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) empath.desktop $(APPSDIR)
	$(mkinstalldirs) $(kde_datadir)/empath
	
uninstall-local:
	-rm -f $(APPSDIR)/empath.desktop
	-rm -rf $(kde_datadir)/empath

