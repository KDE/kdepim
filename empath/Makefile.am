KDE_CXXFLAGS = $(USE_RTTI)
APPSDIR = $(kde_appsdir)/Internet
INCLUDES= $(all_includes) \
	-I$(top_srcdir)/empath/lib/ -I$(top_srcdir)/empath/ui/ -I$(top_srcdir)/rmm/
SUBDIRS = pics import lib ui

bin_PROGRAMS = empath

empath_SOURCES = main.cpp

empath_LDADD = \
	./ui/libempathui.a ./lib/libempath.a -L$(top_srcdir)/rmm/ -lrmm \
	$(LIB_KIO) $(LIB_KFILE) $(LIB_KDEUI) $(LIB_KDECORE)

empath_LDFLAGS = $(all_libraries) $(KDE_RPATH)

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot */*.cpp *.cpp && \
	mv messages.po $(podir)/empath.pot

install-data-local: 
	$(mkinstalldirs) $(APPSDIR)
	$(INSTALL_DATA) $(srcdir)/empath.desktop $(APPSDIR)
	$(mkinstalldirs) $(kde_datadir)/empath
	$(INSTALL_DATA) empath_logo.png $(kde_datadir)/empath/empath_logo.png
	
uninstall-local:
	-rm -f $(APPSDIR)/empath.desktop
	-rm -rf $(kde_datadir)/empath

