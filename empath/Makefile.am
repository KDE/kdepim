KDE_CXXFLAGS = $(USE_RTTI)
APPSDIR = $(kde_appsdir)/Internet
INCLUDES= $(all_includes) -I$(top_srcdir)/empath/rmm/include/ -I$(top_srcdir)/empath/lib/ -I$(top_srcdir)/empath/ui/
SUBDIRS = pics import rmm lib ui

bin_PROGRAMS = empath

empath_SOURCES = main.cpp

empath_LDADD = \
	./ui/libempathui.a ./lib/libempath.a ./rmm/librmm.a \
	$(LIB_KIO) -lkhtml $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE)

empath_LDFLAGS = $(all_libraries) $(KDE_RPATH)

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot */*.cpp *.cpp && \
	mv messages.po ../po/empath.pot

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) $(srcdir)/empath.desktop $(APPSDIR)
	$(mkinstalldirs) $(kde_datadir)/empath
	
uninstall-local:
	-rm -f $(APPSDIR)/empath.desktop
	-rm -rf $(kde_datadir)/empath

